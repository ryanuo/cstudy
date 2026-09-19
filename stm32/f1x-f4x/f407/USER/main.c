#include "stm32f4xx.h"
#include "OLED.h"
#include "LED.h"
#include "DELAY.h"
#include "esp8266.h"

/* ==========================================================================
 * USART3 / ESP8266 链路诊断固件（一次性把"谁的问题"分清）
 *
 * 屏幕上每行含义：
 *   REG:OK  BRR:016D   MCU 侧寄存器自检：GPIOB/USART3 时钟、PB10/PB11 复用模式、
 *                      AFR=AF7、USART3 CR1(UE/TE/RE/RXNEIE)；BRR 是波特率分频值
 *                      （168MHz 下 APB1=42MHz，115200 对应 0x016D）
 *   LOOP:OK n=004      回环自测：拔掉模块、把 UART3 排针的 TXD3 与 RXD3 短接后
 *                      应该显示 OK（收到自己刚发出去的 AT）。
 *                      短接了还是 n=000  -> 问题在 MCU 侧（引脚/中断/代码）
 *                      短接后 OK          -> MCU 侧没问题，问题在模块/座子
 *   115200 / 74880 / 9600   模块在这三个波特率下各回了多少字节（OK = 回了 OK）
 *   下面两行十六进制   第一个有数据的来源的前 20 字节
 *   lost= 缓冲满丢掉的字节数（正常一直为 0）
 *
 * 板子 LED0(PF9，代码 LED1) 亮 = 回环自测通过
 * 板子 LED1(PF10，代码 LED2) 亮 = 某个波特率下模块回了 OK
 * ========================================================================== */

#define HEX_MAX 20

static void OLED_ShowHexLine(int16_t Y, uint8_t *buf, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < 10; i++)
    {
        if (i < len)
            OLED_ShowHexNum((int16_t)(i * 12), Y, buf[i], 2, OLED_6X8);
        else
            OLED_ShowString((int16_t)(i * 12), Y, "  ", OLED_6X8);
    }
}

/* 回显一个波特率的结果：成功显示 OK，失败显示收到的字节数 */
static void OLED_ShowBaudResult(int16_t X, int16_t Y, uint8_t ok, uint16_t n)
{
    if (ok)
        OLED_ShowString(X, Y, "OK ", OLED_6X8);
    else
        OLED_ShowNum(X, Y, n, 3, OLED_6X8);
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
    uint8_t  rx[24];
    uint8_t  dump[HEX_MAX];
    uint16_t dump_len = 0;
    uint16_t n[3] = {0, 0, 0};
    uint8_t  okat[3] = {0, 0, 0};
    uint16_t loop_n = 0;
    uint8_t  loop_ok = 0;
    uint8_t  reg_ok, i, k;
    const uint32_t bauds[3] = {115200, 74880, 9600};

    LED_init();
    ESP8266_Init();
    OLED_Init();
    OLED_Clear();

    OLED_ShowString(0, 0, "ESP8266 DIAG", OLED_8X16);
    OLED_Update();

    /* ---------- 1. 寄存器自检（不需要模块、不需要接线） ---------- */
    reg_ok = RegSelfCheck();
    OLED_ShowString(0, 16, "REG:", OLED_6X8);
    OLED_ShowString(24, 16, reg_ok ? "OK " : "BAD", OLED_6X8);
    OLED_ShowString(54, 16, "BRR:", OLED_6X8);
    OLED_ShowHexNum(78, 16, USART3->BRR, 4, OLED_6X8);
    OLED_Update();

    /* ---------- 2. 回环自测 ---------- */
    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT");
    DELAY_ms(200);
    loop_n = ESP8266_Peek(rx, sizeof(rx));
    for (k = 0; k + 1 < loop_n; k++)
    {
        if (rx[k] == 'A' && rx[k + 1] == 'T') loop_ok = 1;
    }
    if (loop_n > 0)
    {
        dump_len = loop_n;
        for (k = 0; k < loop_n; k++) dump[k] = rx[k];
    }
    if (loop_ok) LED1_on();

    OLED_ShowString(0, 24, "LOOP:", OLED_6X8);
    OLED_ShowString(30, 24, loop_ok ? "OK " : "-- ", OLED_6X8);
    OLED_ShowString(54, 24, "n=", OLED_6X8);
    OLED_ShowNum(66, 24, loop_n, 3, OLED_6X8);
    OLED_Update();

    /* ---------- 3. 三个常见波特率各发一次 AT ---------- */
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
                dump_len = (n[i] > HEX_MAX) ? HEX_MAX : n[i];
                for (k = 0; k < dump_len; k++) dump[k] = rx[k];
            }
        }
    }

    /* ---------- 4. 汇总显示 ---------- */
    OLED_ShowNum(0, 32, 115200, 6, OLED_6X8);
    OLED_ShowBaudResult(42, 32, okat[0], n[0]);
    OLED_ShowNum(66, 32, 74880, 5, OLED_6X8);
    OLED_ShowBaudResult(102, 32, okat[1], n[1]);

    OLED_ShowNum(0, 40, 9600, 4, OLED_6X8);
    OLED_ShowBaudResult(30, 40, okat[2], n[2]);
    OLED_ShowString(60, 40, "lost=", OLED_6X8);
    OLED_ShowNum(90, 40, ESP8266_LostCount(), 3, OLED_6X8);

    OLED_ShowHexLine(48, dump, dump_len > 10 ? 10 : dump_len);
    OLED_ShowHexLine(56, dump + 10, dump_len > 10 ? (dump_len - 10) : 0);
    OLED_Update();

    /* ---------- 5. 停在这里，屏幕保持不动（改完接线按复位键重测） ---------- */
    while (1)
    {
    }
}
