#include "esp8266.h"
#include "string.h"
#include "stdio.h"
#include "stm32f4xx.h"

/*
 * 接收路径分两级：
 *   1) esp_rx_buf  —— USART3 中断里的环形缓冲（只负责把字节接住）
 *   2) esp_acc     —— 主循环的累积文本缓冲（WaitResponse/Peek 都看它）
 *
 * 为什么要有第 2 级：早先的写法是"环形缓冲只读不消费"，每轮循环都把未读数据
 * 整个拷进临时数组再 strstr。一旦模块开始吐数据（AT+CWJAP 会吐一堆进度和 URC），
 * 每轮耗时从约 19 个周期暴涨到上千个周期，用循环次数当超时的 WaitResponse
 * 就会把 15 秒变成十几分钟 —— 表现就是屏幕"卡住"。
 * 现在改成：新字节只追加一次，读走就把 tail 推进（消费掉），循环里没有重复劳动。
 */
static uint8_t  esp_rx_buf[ESP8266_RX_BUF_SIZE];
static volatile uint16_t esp_rx_head = 0;   /* 中断写入位置 */
static volatile uint16_t esp_rx_tail = 0;   /* 读取位置 */
static volatile uint16_t esp_rx_lost = 0;   /* 缓冲满丢弃的字节数 */

#define ESP8266_ACC_SIZE 384
static char     esp_acc[ESP8266_ACC_SIZE];  /* 累积文本（上次 ClearBuffer 之后收到的） */
static uint16_t esp_acc_len = 0;

static volatile uint32_t esp_tick_ms = 0;   /* SysTick 1ms 滴答 */

/* ---------- 时间基准 ---------- */

/**
  * @brief  SysTick 1ms 中断（启动文件里是弱符号，这里覆盖它）
  */
void SysTick_Handler(void)
{
    esp_tick_ms++;
}

/**
  * @brief  启动 1ms 滴答
  */
void ESP8266_TickInit(void)
{
    SysTick_Config(SystemCoreClock / 1000U);
}

/**
  * @brief  取当前滴答（毫秒）
  */
uint32_t ESP8266_GetTick(void)
{
    return esp_tick_ms;
}

/**
  * @brief  基于滴答的真实毫秒延时
  */
void ESP8266_DelayMs(uint32_t ms)
{
    uint32_t start = esp_tick_ms;
    while ((uint32_t)(esp_tick_ms - start) < ms);
}

/* ---------- 接收缓冲 ---------- */

/**
  * @brief  把环形缓冲里"新收到"的字节追加到累积文本缓冲，并推进 tail（消费掉）
  * @note   累积缓冲满时只保留最近 128 字节，避免丢掉"最新那句话"
  */
static void esp_pump(void)
{
    uint16_t head = esp_rx_head;
    uint16_t tail = esp_rx_tail;

    if (head == tail) return;

    while (tail != head)
    {
        if (esp_acc_len >= ESP8266_ACC_SIZE - 1)
        {
            memmove(esp_acc, esp_acc + esp_acc_len - 128, 128);
            esp_acc_len = 128;
        }
        esp_acc[esp_acc_len++] = (char)esp_rx_buf[tail];
        tail = (tail + 1) % ESP8266_RX_BUF_SIZE;
    }
    esp_acc[esp_acc_len] = '\0';
    esp_rx_tail = head;          /* 已消费 */
}

/**
  * @brief  ESP8266 硬件初始化 (USART3 + GPIO)
  *         接线（GEC-M4 板 P7 座 / UART3 排针）：模块 VCC=3.3V、GND=GND、
  *         模块 TXD -> PB11(RXD3)、模块 RXD -> PB10(TXD3)、模块 EN -> 3.3V
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

    /* 6. 起 1ms 滴答（超时/延时都靠它） */
    ESP8266_TickInit();
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
  * @brief  清空接收缓冲（发下一条命令前调用）
  */
void ESP8266_ClearBuffer(void)
{
    esp_rx_tail = esp_rx_head;
    esp_acc_len = 0;
    esp_acc[0] = '\0';
    esp_rx_lost = 0;
}

/**
  * @brief  等待特定响应（真实毫秒超时）
  * @note   匹配成功后不清缓冲：调用方发命令前会 ClearBuffer，
  *         这样成功后的 Peek() 还能把模块的回复（例如 IP）显示出来。
  */
uint8_t ESP8266_WaitResponse(char *expected, uint32_t timeout_ms)
{
    uint32_t start = esp_tick_ms;

    while ((uint32_t)(esp_tick_ms - start) < timeout_ms)
    {
        if (esp_rx_head != esp_rx_tail)
        {
            esp_pump();
            if (strstr(esp_acc, expected) != NULL) return 1;
        }
    }
    return 0;
}

/**
  * @brief  目前累积收到的数据里是否包含 expected（不消费）
  */
uint8_t ESP8266_Contains(char *expected)
{
    esp_pump();
    return (strstr(esp_acc, expected) != NULL) ? 1 : 0;
}

/**
  * @brief  看一眼目前累积收到的原始数据（不消费），用于显示/诊断
  * @retval 实际复制到的字节数（0 = 一个字节都没收到）
  */
uint16_t ESP8266_Peek(uint8_t *dst, uint16_t max_len)
{
    uint16_t len;

    esp_pump();
    len = esp_acc_len;
    if (len > max_len) len = max_len;
    memcpy(dst, esp_acc, len);
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
