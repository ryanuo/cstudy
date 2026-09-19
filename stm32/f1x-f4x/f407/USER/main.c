#include "stm32f4xx.h"
#include "OLED.h"
#include "LED.h"
#include "DELAY.h"
#include "esp8266.h"

/* ==========================================================================
 * USART3 / ESP8266 链路诊断固件 v3（尽量不用动线）
 *
 * 屏幕每行含义：
 *   REG:OK  BRR:016C  MCU 侧寄存器自检：GPIOB/USART3 时钟、PB10/PB11 复用模式、
 *                     AFR=AF7、USART3 CR1；BRR 是波特率分频值
 *                     （APB1=42MHz，115200 对应 0x16C~0x16D）
 *   SELF:OK           【片内回环自测】把 CR3 的 HDSEL 置 1，TX 与 RX 在芯片内部
 *                     相连（参考手册 26.3.10）。不需要拔模块、不需要接线。
 *                     OK = USART3 发送/接收 + RXNE 中断 + NVIC + 缓冲 + 匹配
 *                     整条链路都是好的，也就是 MCU 侧代码没问题。
 *   PIN:1             PB11 临时配成下拉输入读到的电平。
 *                     1 = 线上有东西主动拉高（模块 TX 空闲就是高）-> 线通且有供电
 *                     0 = 悬空/没驱动 -> 模块没供电、被按住复位，或者线断
 *   BOOT:040          【冷启动抓到的字节数】上电后头 1.5 秒用 74880 监听模块的
 *                     启动信息。必须冷启动（关电源开关再开），按 MCU 复位键
 *                     不算 —— 模块不会跟着重启，它早就启动完了。
 *   LOOP:000          外部回环：拔掉模块 + 短接 UART3 排针的 TXD3/RXD3 后的字节数
 *   115200/74880/9600 模块在这三个波特率下各回了多少字节（OK = 回了 OK）
 *   下面两行 ASCII    第一个有数据的来源的前 40 个字符（不可打印显示为 .）
 *
 * 板子 LED0(PF9，代码 LED1) 亮 = 片内回环通过
 * 板子 LED1(PF10，代码 LED2) 亮 = 某个波特率下模块回了 OK
 * ========================================================================== */

#define DUMP_MAX 40
#define LINE_MAX 20

static void OLED_ShowAscii(int16_t Y, uint8_t *buf, uint16_t len)
{
    char s[LINE_MAX + 2];
    uint16_t i;

    for (i = 0; i < LINE_MAX; i++)
    {
        if (i < len && buf[i] >= 0x20 && buf[i] <= 0x7E)
            s[i] = (char)buf[i];
        else if (i < len)
            s[i] = '.';
        else
            s[i] = ' ';
    }
    s[LINE_MAX] = '\0';
    OLED_ShowString(0, Y, s, OLED_6X8);
}

/* 画 "标签:结果"，结果要么是 OK 要么是 3 位数字 */
static void OLED_ShowResult(int16_t X, int16_t Y, uint8_t ok, uint16_t n)
{
    if (ok)
        OLED_ShowString(X, Y, "OK ", OLED_6X8);
    else
        OLED_ShowNum(X, Y, (n > 999) ? 999 : n, 3, OLED_6X8);
}

/* MCU 侧寄存器自检 */
static uint8_t RegSelfCheck(void)
{
    uint8_t ok = 1;

    if ((RCC->AHB1ENR & RCC_AHB1ENR_GPIOBEN) == 0)   ok = 0;   /* GPIOB 时钟 */
    if ((RCC->APB1ENR & RCC_APB1ENR_USART3EN) == 0)  ok = 0;   /* USART3 时钟 */
    if (((GPIOB->MODER >> 20) & 0x3) != 0x2)         ok = 0;   /* PB10 复用模式 */
    if (((GPIOB->MODER >> 22) & 0x3) != 0x2)         ok = 0;   /* PB11 复用模式 */
    if (((GPIOB->AFR[1] >> 8)  & 0xF) != 7)          ok = 0;   /* PB10 = AF7 */
    if (((GPIOB->AFR[1] >> 12) & 0xF) != 7)          ok = 0;   /* PB11 = AF7 */
    if ((USART3->CR1 & (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE))
        != (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE)) ok = 0;
    return ok;
}

int main(void)
{
    uint8_t  rx[DUMP_MAX];
    uint8_t  dump[DUMP_MAX];
    uint16_t dump_len = 0;
    uint16_t n[3] = {0, 0, 0};
    uint8_t  okat[3] = {0, 0, 0};
    uint16_t boot_n = 0, loop_n = 0;
    uint8_t  self_ok, pin_lvl, reg_ok, i, k;
    const uint32_t bauds[3] = {115200, 74880, 9600};

    LED_init();
    ESP8266_Init();
    ESP8266_SetBaud(74880);        /* 先切到 74880，抢在模块启动信息之前开始听 */
    ESP8266_ClearBuffer();

    OLED_Init();
    OLED_Clear();
    OLED_ShowString(0, 0, "ESP8266 LINK DIAG", OLED_6X8);
    OLED_ShowString(0, 8, "listen boot 74880", OLED_6X8);
    OLED_Update();

    /* ---------- 1. 冷启动后头 1.5 秒：抓模块的启动信息（74880） ---------- */
    DELAY_ms(1500);
    boot_n = ESP8266_Peek(rx, sizeof(rx));
    if (boot_n > 0)
    {
        dump_len = boot_n;
        for (k = 0; k < dump_len; k++) dump[k] = rx[k];
    }
    OLED_ShowString(0, 8, "                 ", OLED_6X8);   /* 清提示行 */

    /* ---------- 2. REG 寄存器自检 ---------- */
    reg_ok = RegSelfCheck();
    OLED_ShowString(0, 8, "REG:", OLED_6X8);
    OLED_ShowString(24, 8, reg_ok ? "OK " : "BAD", OLED_6X8);
    OLED_ShowString(54, 8, "BRR:", OLED_6X8);
    OLED_ShowHexNum(78, 8, USART3->BRR, 4, OLED_6X8);
    OLED_Update();

    /* ---------- 3. PIN 探测 + 片内回环自测（都不需要动线） ---------- */
    pin_lvl = ESP8266_ProbeRxPin();
    ESP8266_SetBaud(115200);
    self_ok = ESP8266_SelfLoopTest("AT", 300);
    if (self_ok) LED1_on();

    OLED_ShowString(0, 16, "SELF:", OLED_6X8);
    OLED_ShowString(30, 16, self_ok ? "OK " : "-- ", OLED_6X8);
    OLED_ShowString(54, 16, "PIN:", OLED_6X8);
    OLED_ShowNum(78, 16, pin_lvl, 1, OLED_6X8);
    OLED_Update();

    /* ---------- 4. 外部回环（拔模块 + 短接 TXD3/RXD3） ---------- */
    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT");
    DELAY_ms(200);
    loop_n = ESP8266_Peek(rx, sizeof(rx));
    if (loop_n > 0 && dump_len == 0)
    {
        dump_len = loop_n;
        for (k = 0; k < dump_len; k++) dump[k] = rx[k];
    }

    OLED_ShowString(0, 24, "BOOT:", OLED_6X8);
    OLED_ShowNum(30, 24, (boot_n > 999) ? 999 : boot_n, 3, OLED_6X8);
    OLED_ShowString(54, 24, "LOOP:", OLED_6X8);
    OLED_ShowNum(84, 24, (loop_n > 999) ? 999 : loop_n, 3, OLED_6X8);
    OLED_Update();

    /* ---------- 5. 三个常见波特率各发一次 AT ---------- */
    for (i = 0; i < 3; i++)
    {
        ESP8266_SetBaud(bauds[i]);
        ESP8266_ClearBuffer();
        ESP8266_SendAT("AT");

        if (ESP8266_WaitResponse("OK", 1000))
        {
            okat[i] = 1;
            LED2_on();
        }
        else
        {
            n[i] = ESP8266_Peek(rx, sizeof(rx));
            if (n[i] > 0 && dump_len == 0)
            {
                dump_len = n[i];
                for (k = 0; k < dump_len; k++) dump[k] = rx[k];
            }
        }
    }

    /* ---------- 6. 汇总显示 ---------- */
    OLED_ShowNum(0, 32, 115200, 6, OLED_6X8);
    OLED_ShowResult(42, 32, okat[0], n[0]);
    OLED_ShowNum(66, 32, 74880, 5, OLED_6X8);
    OLED_ShowResult(102, 32, okat[1], n[1]);

    OLED_ShowNum(0, 40, 9600, 4, OLED_6X8);
    OLED_ShowResult(30, 40, okat[2], n[2]);
    OLED_ShowString(60, 40, "lost=", OLED_6X8);
    OLED_ShowNum(90, 40, ESP8266_LostCount(), 3, OLED_6X8);

    OLED_ShowAscii(48, dump, (dump_len > LINE_MAX) ? LINE_MAX : dump_len);
    OLED_ShowAscii(56, dump + LINE_MAX, (dump_len > LINE_MAX) ? (dump_len - LINE_MAX) : 0);
    OLED_Update();

    /* ---------- 7. 停在这里，屏幕保持不动（改完接线/冷启动后重测） ---------- */
    while (1)
    {
    }
}
