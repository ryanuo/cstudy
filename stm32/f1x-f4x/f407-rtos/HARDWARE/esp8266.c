#include "esp8266.h"
#include "string.h"
#include "stm32f4xx.h"

/*
 * 接收路径分两级：
 *   1) esp_rx_buf  —— USART3 中断里的环形缓冲（只负责把字节接住）
 *   2) esp_acc     —— 主循环的累积文本缓冲（WaitResponse/Contains/Find/Peek 都看它）
 *
 * 新字节只追加一次，读走就把 tail 推到 head（消费掉）；超时用 SysTick 滴答计真实
 * 毫秒，不数循环次数 —— 否则一旦模块开始吐数据，循环每轮耗时暴涨，超时会被拖长很多倍。
 */
static uint8_t  esp_rx_buf[ESP8266_RX_BUF_SIZE];
static volatile uint16_t esp_rx_head = 0;   /* 中断写入位置 */
static volatile uint16_t esp_rx_tail = 0;   /* 读取位置 */

#define ESP8266_ACC_SIZE 2048   /* 多人排队时能多装几个请求 */
static char     esp_acc[ESP8266_ACC_SIZE];  /* 累积文本（上次 ClearBuffer 之后收到的） */
static uint16_t esp_acc_len = 0;

static volatile uint32_t esp_tick_ms = 0;

/* ---------- 时间基准 ---------- */

void SysTick_Handler(void)          /* 启动文件里是弱符号，这里覆盖它 */
{
    esp_tick_ms++;
}

void ESP8266_TickInit(void)
{
    SysTick_Config(SystemCoreClock / 1000U);
}

uint32_t ESP8266_GetTick(void)
{
    return esp_tick_ms;
}

void ESP8266_DelayMs(uint32_t ms)
{
    uint32_t start = esp_tick_ms;
    while ((uint32_t)(esp_tick_ms - start) < ms);
}

/* ---------- 接收缓冲 ---------- */

/**
  * @brief  把环形缓冲里新收到的字节追加到累积文本缓冲（并推进 tail）
  * @note   累积缓冲满时只保留最近 128 字节，避免丢掉最新那句话
  */
static void esp_pump(void)
{
    uint16_t head = esp_rx_head;
    uint16_t tail = esp_rx_tail;

    if (head == tail) return;

    while (tail != head)
    {
        if (esp_acc_len < ESP8266_ACC_SIZE - 1)
        {
            esp_acc[esp_acc_len++] = (char)esp_rx_buf[tail];
        }
        /* 满了就丢掉后面的新字节，保住开头 —— HTTP 请求行/AT 回复的头都在前面 */
        tail = (tail + 1) % ESP8266_RX_BUF_SIZE;
    }
    esp_acc[esp_acc_len] = '\0';
    esp_rx_tail = head;
}

/**
  * @brief  初始化：USART3 (PB10=TX, PB11=RX, 115200-8-N-1) + RXNE 中断 + 1ms 滴答
  *         接线：模块 VCC->3.3V、GND->GND、模块 TXD->PB11、模块 RXD->PB10、模块 EN->3.3V
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
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    /* 3. 配置 USART3 (115200-8-N-1) */
    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    /* 4. NVIC 接收中断 */
    NVIC_InitStructure.NVIC_IRQChannel                   = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 5. 使能接收中断和串口 */
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);

    /* 6. 1ms 滴答 */
    ESP8266_TickInit();
}

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

void ESP8266_SendData(uint8_t *data, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++) {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3, data[i]);
    }
}

void ESP8266_ClearBuffer(void)
{
    esp_rx_tail = esp_rx_head;
    esp_acc_len = 0;
    esp_acc[0] = '\0';
}

/* 只清 AT 的回应噪声（OK / > / SEND OK / ERROR 之类），把还在排队的 +IPD 请求块整块留下。
   为什么不能直接 ClearBuffer：回复一个请求要几十~几百毫秒，这期间浏览器已经把下一个请求发过来了
   （响应带 Connection: close，浏览器必然开新连接）。ClearBuffer 连"还没搬走的环形缓冲字节"一起丢，
   那个请求就永远没人处理 —— 表现是"第一次能拿到数据，之后再请求就没反应"；
   而且没人关的连接会占满模块的 5 个 link 槽位，最后连第一次请求都超时。 */
void ESP8266_ClearNonIp(void)
{
    char     keep[ESP8266_ACC_SIZE];
    uint16_t klen = 0;
    char    *p = esp_acc;

    esp_pump();                     /* 先把环形缓冲里已经到的字节搬过来，别丢 */
    while (*p)
    {
        char    *q = strstr(p, "+IPD,"), *h, *d;
        uint16_t id = 0, len = 0, total, have;

        if (q == 0) break;          /* 后面没有请求块了，剩下的都是 AT 噪声 */

        h = q + 5;
        while (*h >= '0' && *h <= '9') { id = (uint16_t)(id * 10 + (uint16_t)(*h - '0')); h++; }
        if (*h == ',') { h++; while (*h >= '0' && *h <= '9') { len = (uint16_t)(len * 10 + (uint16_t)(*h - '0')); h++; } }
        else           { len = id; }

        if (*h != ':') break;       /* 头还没收全，整块留到下一轮 */
        d = h + 1;

        have = (uint16_t)((esp_acc + esp_acc_len) - d);
        if (have < len)             /* 数据没收全：头和已到的部分先留着 */
        {
            if ((uint16_t)(klen + (d - q) + have) < ESP8266_ACC_SIZE)
            {
                memcpy(keep + klen, q, (size_t)((d - q) + have));
                klen = (uint16_t)(klen + (d - q) + have);
            }
            break;
        }

        total = (uint16_t)((d - q) + len);
        if ((uint16_t)(klen + total) >= ESP8266_ACC_SIZE) break;
        memcpy(keep + klen, q, (size_t)total);
        klen = (uint16_t)(klen + total);
        p = d + len;
    }

    memcpy(esp_acc, keep, (size_t)klen);
    esp_acc_len = klen;
    esp_acc[klen] = '\0';
}

/* 关掉所有已建立的链接（多连接模式里 link id=5 表示全部）。
   模块的 link 槽位只有 5 个，被残连接占满后就再也不 accept 新连接 —— 这是"服务器哑掉"的常见原因 */
uint8_t ESP8266_CloseAllLinks(void)
{
    ESP8266_ClearNonIp();           /* 别把刚到的请求清丢了 */
    ESP8266_SendAT("AT+CIPCLOSE=5");
    return ESP8266_WaitResponse("OK", 1000);
}

/* ---------- 按块取请求：只消费自己这一块，别人排队的请求留着 ---------- */

/* 丢掉 mark 之前的已处理内容 */
static void esp_drop_to(char *mark)
{
    uint16_t off;

    if (mark == 0 || mark <= esp_acc || mark >= esp_acc + esp_acc_len)
    {
        esp_acc_len = 0;
        esp_acc[0]   = '\0';
        return;
    }

    off = (uint16_t)(mark - esp_acc);
    memmove(esp_acc, esp_acc + off, (size_t)(esp_acc_len - off));
    esp_acc_len = (uint16_t)(esp_acc_len - off);
    esp_acc[esp_acc_len] = '\0';
}

/**
  * @brief  取出缓冲里第一个 "+IPD,<id>,<len>:<数据>" 块：拷进 buf 并 NUL 结尾，
  *         取完立刻把这一块消费掉（后面排队的请求原样保留）
  * @note   数据还没收全就返回 0，等下一轮 —— 请求行在数据最前面，
  *         所以即使 max 截断了尾巴也不影响解析
  * @retval 数据长度；0 = 还没有完整的一块
  */
uint16_t ESP8266_TakeIp(uint8_t *pid, char *buf, uint16_t max)
{
    char    *p, *q;
    uint16_t id = 0, len = 0, n;

    p = strstr(esp_acc, "+IPD,");
    if (p == 0) return 0;

    q = p + 5;
    while (*q >= '0' && *q <= '9') { id = (uint16_t)(id * 10 + (uint16_t)(*q - '0')); q++; }

    if (*q == ',')                       /* 多链接："+IPD,<id>,<len>:" */
    {
        q++;
        while (*q >= '0' && *q <= '9') { len = (uint16_t)(len * 10 + (uint16_t)(*q - '0')); q++; }
    }
    else                                 /* 单链接："+IPD,<len>:" */
    {
        len = id;
        id  = 0;
    }

    if (*q != ':' || len == 0) return 0;                            /* 头还没收全 */
    q++;
    if ((uint16_t)((esp_acc + esp_acc_len) - q) < len) return 0;    /* 数据还没收全 */

    n = (uint16_t)((len < (uint16_t)(max - 1)) ? len : (uint16_t)(max - 1));
    memcpy(buf, q, (size_t)n);
    buf[n] = '\0';
    *pid   = (uint8_t)id;

    esp_drop_to(q + len);                /* 只消费这一块 */
    return n;
}


/**
  * @brief  等待特定响应（真实毫秒超时）
  * @note   匹配成功后不清缓冲：调用方发命令前会 ClearBuffer，
  *         这样成功后的 Peek()/Find() 还能把模块的回复（例如 IP）读出来
  */
uint8_t ESP8266_WaitResponse(char *expected, uint32_t timeout_ms)
{
    uint32_t start = esp_tick_ms;

    while ((uint32_t)(esp_tick_ms - start) < timeout_ms)
    {
        if (esp_rx_head != esp_rx_tail)
        {
            esp_pump();
            if (strstr(esp_acc, expected) != 0) return 1;

            /* ERROR / FAIL / CLOSED 说明这次没戏了，不用再等满超时。
               以前等满 1 秒，网页点一下就会卡一秒（CIPCLOSE 时最常见）*/
            if (strstr(esp_acc, "ERROR")  != 0) return 0;
            if (strstr(esp_acc, "FAIL")   != 0) return 0;
            if (strstr(esp_acc, "CLOSED") != 0) return 0;
        }
    }
    return 0;
}

uint8_t ESP8266_Contains(char *expected)
{
    esp_pump();
    return (strstr(esp_acc, expected) != 0) ? 1 : 0;
}

char *ESP8266_Find(char *pattern)
{
    esp_pump();
    return strstr(esp_acc, pattern);
}

/**
  * @brief  从累积文本里抓一个 IPv4 地址（形如 192.168.1.101）
  * @note   不同 AT 固件 AT+CIFSR 的返回格式不一样：新版是
  *         "+CIFSR:STAIP,"192.168.1.101""，老版直接返回裸的 "192.168.1.101"。
  *         这里按"数字.数字.数字.数字"找，取最后一个（APIP 在前、STAIP 在后；
  *         老版只有一个），两种格式都能拿到。
  * @retval 1 = 抓到了；0 = 没抓到
  */
uint8_t ESP8266_FindIp(char *dst, uint8_t max_len)
{
    char *p = esp_acc;
    char *found = 0;
    uint8_t found_len = 0, k;

    esp_pump();

    while (*p != '\0')
    {
        char *q = p;
        uint8_t seg, digits, ok = 1;

        if (!(*p >= '0' && *p <= '9')) { p++; continue; }

        for (seg = 0; seg < 4 && ok; seg++)
        {
            digits = 0;
            while (*q >= '0' && *q <= '9' && digits < 3) { q++; digits++; }
            if (digits == 0) { ok = 0; break; }
            if (seg < 3)
            {
                if (*q != '.') { ok = 0; break; }
                q++;
            }
        }
        if (ok && !(*q >= '0' && *q <= '9'))   /* 后面不能再接数字，避免只匹配到长数字的前缀 */
        {
            found = p;
            found_len = (uint8_t)(q - p);
            p = q;
        }
        else
        {
            p++;
        }
    }

    if (found == 0) { dst[0] = '\0'; return 0; }

    k = 0;
    while (k < found_len && k < (uint8_t)(max_len - 1)) { dst[k] = found[k]; k++; }
    dst[k] = '\0';
    return 1;
}

uint16_t ESP8266_Peek(uint8_t *dst, uint16_t max_len)
{
    uint16_t len;

    esp_pump();
    len = esp_acc_len;
    if (len > max_len) len = max_len;
    memcpy(dst, esp_acc, len);
    return len;
}

void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        uint8_t data = USART_ReceiveData(USART3);
        uint16_t next_head = (esp_rx_head + 1) % ESP8266_RX_BUF_SIZE;

        if (next_head != esp_rx_tail)      /* 缓冲满就丢掉这个字节，不让缓冲区错乱 */
        {
            esp_rx_buf[esp_rx_head] = data;
            esp_rx_head = next_head;
        }
    }
}
