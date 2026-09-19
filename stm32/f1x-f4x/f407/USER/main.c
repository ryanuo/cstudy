#include "stm32f4xx.h"
#include "OLED.h"
#include "LED.h"
#include "esp8266.h"

/* ==========================================================================
 * ESP8266 (ESP-01S, AT 固件) 连 WiFi
 *
 * 板子丝印 与 代码函数 的对应（GEC-M4 原理图 02-KEY_LED 页，低电平点亮）：
 *   板子 LED0     = PF9  = LED1_on()/LED1_off()   <-- 代码里叫 LED1
 *   板子 LED1     = PF10 = LED2_on()/LED2_off()   <-- 代码里叫 LED2
 *   板子 FSMC_D10 = PE13 = LED3_on()/LED3_off()
 *   板子 FSMC_D11 = PE14 = LED4_on()/LED4_off()
 *
 * 灯语：
 *   板子 LED0 (PF9) 亮 = AT 通了
 *   板子 LED1 (PF10) 亮 = AT+CWMODE=1 成功
 *   PE13 亮 = 连上热点并拿到 IP
 *   板子 LED1 慢闪 = AT 没通；快闪 = AT 通了但 WiFi 没连上（每 10 秒自动重试）
 *
 * 接线：模块 VCC->3.3V（不要接排针上的 VCC5V）、GND->GND、
 *       模块 TXD->PB11(RXD3)、模块 RXD->PB10(TXD3)、模块 EN->3.3V，
 *       VCC-GND 间并 100uF 抗发射瞬态跌落。
 * ========================================================================== */

#define WIFI_SSID     "YQ-shixun5"   /* 必须是 2.4G 热点，大小写要和热点一致 */
#define WIFI_PASS     "88888888"
#define AT_RETRY_NUM  10

static uint8_t wifi_ok = 0;

/* 把收到的原始数据按 ASCII 显示（不可打印字符显示为 .），一行 20 个字 */
static void OLED_ShowAscii(int16_t Y, uint8_t *buf, uint16_t len)
{
    char s[22];
    uint16_t i;

    for (i = 0; i < 20; i++)
    {
        if (i < len && buf[i] >= 0x20 && buf[i] <= 0x7E)
            s[i] = (char)buf[i];
        else if (i < len)
            s[i] = '.';
        else
            s[i] = ' ';
    }
    s[20] = '\0';
    OLED_ShowString(0, Y, s, OLED_6X8);
}

/* 最多拷 max_len-1 个字符并补 '\0'
   （OLED_ShowString 会一直画到 '\0'，从累积缓冲拿到的指针必须先截断） */
static void CopyN(char *dst, char *src, uint8_t max_len)
{
    uint8_t k = 0;

    if (src == 0) { dst[0] = '\0'; return; }
    while (k < (uint8_t)(max_len - 1) && src[k] != '\0') { dst[k] = src[k]; k++; }
    dst[k] = '\0';
}

/* 连 WiFi：15 秒内等到 "GOT IP" 或 "OK" 算成功；FAIL/ERROR 立刻返回 */
static uint8_t WIFI_Connect(void)
{
    uint32_t start;

    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CWJAP=\"" WIFI_SSID "\",\"" WIFI_PASS "\"");

    start = ESP8266_GetTick();
    while ((uint32_t)(ESP8266_GetTick() - start) < 15000)
    {
        if (ESP8266_Contains("GOT IP") || ESP8266_Contains("OK"))    return 1;
        if (ESP8266_Contains("FAIL")   || ESP8266_Contains("ERROR")) return 0;
    }
    return 0;
}

/* 读 IP 显示在最后一行 */
static void OLED_ShowIp(void)
{
    char line[24];

    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CIFSR");
    ESP8266_WaitResponse("OK", 3000);
    CopyN(line, ESP8266_Find("STAIP"), sizeof(line));
    OLED_ShowString(0, 56, line[0] ? line : "no IP               ", OLED_6X8);
}

int main(void)
{
    uint8_t  at_ok = 0, mode_ok = 0, i, tick = 0;
    uint8_t  rxb[24];
    uint16_t rxn;

    LED_init();
    ESP8266_Init();          /* 里面把 USART3 和 1ms 滴答都起好 */
    OLED_Init();
    OLED_Clear();

    OLED_ShowString(0, 0, "ESP8266 WIFI", OLED_8X16);
    OLED_ShowString(0, 16, "boot 1.5s           ", OLED_6X8);
    OLED_ShowString(0, 40, "SSID:" WIFI_SSID, OLED_6X8);
    OLED_Update();

    ESP8266_DelayMs(1500);   /* ESP-01S 上电要 300ms~1s 才认 AT */

    /* ---------- 1. 循环发 AT ---------- */
    for (i = 1; i <= AT_RETRY_NUM; i++)
    {
        ESP8266_ClearBuffer();
        ESP8266_SendAT("AT");
        if (ESP8266_WaitResponse("OK", 1000)) { at_ok = 1; break; }
    }

    if (at_ok == 0)
    {
        OLED_ShowString(0, 16, "AT FAILED!          ", OLED_6X8);
        OLED_ShowString(0, 24, "check jumper/pwr    ", OLED_6X8);
        rxn = ESP8266_Peek(rxb, 20);
        OLED_ShowAscii(56, rxb, (rxn > 20) ? 20 : rxn);
        OLED_Update();
        while (1) { LED2_on(); ESP8266_DelayMs(300); LED2_off(); ESP8266_DelayMs(300); }
    }

    LED1_on();                                        /* 板子 LED0 (PF9) 亮 */
    OLED_ShowString(0, 16, "AT: OK              ", OLED_6X8);

    /* ---------- 2. Station 模式 ---------- */
    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CWMODE=1");
    mode_ok = ESP8266_WaitResponse("OK", 2000);
    OLED_ShowString(0, 24, mode_ok ? "MODE: OK            " : "MODE: FAIL          ", OLED_6X8);
    if (mode_ok) LED2_on();                           /* 板子 LED1 (PF10) 亮 */

    /* ---------- 3. 连 WiFi ---------- */
    OLED_ShowString(0, 32, "WIFI: connecting... ", OLED_6X8);
    OLED_Update();

    wifi_ok = WIFI_Connect();
    if (wifi_ok)
    {
        LED3_on();                                    /* PE13 亮 = 连上并拿到 IP */
        OLED_ShowString(0, 32, "WIFI: OK            ", OLED_6X8);
        OLED_ShowIp();
    }
    else
    {
        OLED_ShowString(0, 32, "WIFI: FAIL          ", OLED_6X8);
        rxn = ESP8266_Peek(rxb, 20);
        OLED_ShowAscii(48, rxb, (rxn > 20) ? 20 : rxn);
        OLED_ShowString(0, 56, "                    ", OLED_6X8);
    }
    OLED_Update();

    /* ---------- 4. 失败就每 10 秒自动重试 ---------- */
    while (1)
    {
        if (wifi_ok == 0)
        {
            LED2_on();
            ESP8266_DelayMs(150);
            LED2_off();
            ESP8266_DelayMs(850);

            tick++;
            if (tick >= 10)
            {
                tick = 0;
                if (WIFI_Connect())
                {
                    wifi_ok = 1;
                    LED2_on();
                    LED3_on();
                    OLED_ShowString(0, 32, "WIFI: OK            ", OLED_6X8);
                    OLED_ShowIp();
                    OLED_Update();
                }
            }
        }
        else
        {
            ESP8266_DelayMs(500);
        }
    }
}
