#include "esp8266.h"
#include "string.h"
#include "stdio.h"
#include "stm32f4xx.h"

/*
 * 接收缓冲：USART3 中断里写，主循环里读。
 * 本驱动采用"只读不消费"策略：读的时候只把未读数据拷出来做字符串匹配，
 * 靠 ESP8266_ClearBuffer() 复位，这样跨包的 "OK" 也能匹配上。
 */
static uint8_t  esp_rx_buf[ESP8266_RX_BUF_SIZE];
static volatile uint16_t esp_rx_head = 0;   /* 中断写入位置 */
static volatile uint16_t esp_rx_tail = 0;   /* 读取起点 */
static volatile uint16_t esp_rx_lost = 0;   /* 缓冲满丢弃的字节数 */

/* 主循环用的快照缓冲：放静态区，避免在栈上开 512 字节
   （启动文件 Stack_Size 只有 0x400 = 1KB） */
static uint8_t  esp_snap_buf[ESP8266_RX_BUF_SIZE];
static uint16_t esp_snap_len = 0;

/**
  * @brief  把当前未读数据复制到快照缓冲并补 '\0'，返回字节数（不消费）
  */
static uint16_t esp_take_snapshot(void)
{
    uint16_t head = esp_rx_head;   /* 先取一次，避免复制途中被中断改动 */
    uint16_t tail = esp_rx_tail;
    uint16_t i = 0;

    while (tail != head && i < ESP8266_RX_BUF_SIZE - 1)
    {
        esp_snap_buf[i++] = esp_rx_buf[tail];
        tail = (tail + 1) % ESP8266_RX_BUF_SIZE;
    }
    esp_snap_buf[i] = '\0';
    esp_snap_len = i;
    return i;
}

/**
  * @brief  ESP8266 硬件初始化 (USART3 + GPIO)
  *         接线（GEC-M4 板 P7 座 / UART3 排针，原理图 04-WIRELESS 页）：
  *         STM32 PB10(TX) -> 模块 RX，PB11(RX) <- 模块 TX，共地，3.3V
  */
void ESP8266_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 1. 使能时钟 (USART3 在 APB1，GPIOB 在 AHB1) */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    /* 2. 配置 GPIO (PB10=TX, PB11=RX) */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 复用功能映射 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    /* 3. 配置 USART3 (115200-8-N-1，ESP8266 AT 固件默认波特率) */
    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    /* 4. 配置 NVIC 接收中断 */
    NVIC_InitStructure.NVIC_IRQChannel                   = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 5. 使能接收中断和串口 */
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);
}

/**
  * @brief  重新设置 USART3 波特率（诊断用：扫描模块真实波特率）
  * @note   USART_Init 会按 CR1_CLEAR_MASK 清 CR1，会把 RXNEIE 一起清掉，
  *         所以这里必须重新使能接收中断，否则改完波特率就再也收不到数据。
  */
void ESP8266_SetBaud(uint32_t baud)
{
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate            = baud;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);   /* 保住接收中断 */
    USART_Cmd(USART3, ENABLE);
}

/**
  * @brief  片内回环自测（CR3 的 HDSEL 位）
  * @note   参考手册 RM0090 26.3.10：HDSEL 置 1 后 "TX 和 RX 线路从内部相连接"。
  *         所以这条路测的是芯片 + 代码（USART3 发送/接收、RXNE 中断、NVIC、
  *         环形缓冲、快照、字符串匹配），完全不需要外部接线。
  */
uint8_t ESP8266_SelfLoopTest(char *expected, uint32_t timeout_ms)
{
    uint8_t ok;

    USART3->CR3 |= USART_CR3_HDSEL;    /* TX 内部接到 RX */
    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT");
    ok = ESP8266_WaitResponse(expected, timeout_ms);
    USART3->CR3 &= (uint16_t)(~USART_CR3_HDSEL);   /* 恢复全双工 */
    ESP8266_ClearBuffer();
    return ok;
}

/**
  * @brief  探测 PB11 电平（下拉输入）
  * @retval 1 = 线上有东西在主动拉高（模块 TX 空闲应为高电平，说明线通且有供电）
  *         0 = 悬空或没被驱动（模块没供电/被复位按住/线断）
  */
uint8_t ESP8266_ProbeRxPin(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    volatile uint32_t i;
    uint8_t lvl;

    USART_Cmd(USART3, DISABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    for (i = 0; i < 200000; i++);            /* 等电平稳定 */
    lvl = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);

    /* 恢复复用功能 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    USART_Cmd(USART3, ENABLE);
    return lvl;
}

/**
  * @brief  发送 AT 指令 (自动添加 \r\n)
  */
void ESP8266_SendAT(char *cmd)
{
    while (*cmd) {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3, *cmd++);
    }
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    USART_SendData(USART3, '\r');
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    USART_SendData(USART3, '\n');
}

/**
  * @brief  发送原始数据 (透传模式使用)
  */
void ESP8266_SendData(uint8_t *data, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++) {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3, data[i]);
    }
}

/**
  * @brief  清空接收缓冲区（丢弃已收到的数据）
  */
void ESP8266_ClearBuffer(void)
{
    esp_rx_tail = esp_rx_head;   /* tail 追上 head 即为空，比直接清零更安全 */
    esp_rx_lost = 0;
}

/**
  * @brief  等待特定响应 (超时机制)
  * @note   超时计数是按 168MHz 下约 19 个时钟/次空循环标定的，
  *         timeout_ms = 1000 实际约 1.1 秒。
  */
uint8_t ESP8266_WaitResponse(char *expected, uint32_t timeout_ms)
{
    uint32_t delay_cnt = 0;

    while (delay_cnt < timeout_ms * 10000)
    {
        delay_cnt++;

        if (esp_rx_head != esp_rx_tail)
        {
            esp_take_snapshot();
            if (strstr((char *)esp_snap_buf, expected) != NULL)
            {
                ESP8266_ClearBuffer();
                return 1;
            }
        }
    }
    return 0;
}

/**
  * @brief  看一眼当前收到的原始数据（不消费），用于诊断
  * @param  dst     输出缓冲
  * @param  max_len 最多复制多少字节
  * @retval 实际复制到的字节数（0 = 一个字节都没收到）
  */
uint16_t ESP8266_Peek(uint8_t *dst, uint16_t max_len)
{
    uint16_t len = esp_take_snapshot();

    if (len > max_len) len = max_len;
    memcpy(dst, esp_snap_buf, len);
    return len;
}

/**
  * @brief  缓冲满被丢弃的字节数（诊断用，正常应为 0）
  */
uint16_t ESP8266_LostCount(void)
{
    return esp_rx_lost;
}

/**
  * @brief  USART3 中断服务函数
  */
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        uint8_t data = USART_ReceiveData(USART3);

        uint16_t next_head = (esp_rx_head + 1) % ESP8266_RX_BUF_SIZE;
        if (next_head != esp_rx_tail) {
            esp_rx_buf[esp_rx_head] = data;
            esp_rx_head = next_head;
        } else {
            esp_rx_lost++;   /* 缓冲满，丢掉并计数（正常不该发生） */
        }
    }
}
