#include "stm32f4xx.h"
#include "OLED.h"
#include "LED.h"
#include "DELAY.h"
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
 *   板子 LED0 (PF9)  常亮 = 收到 AT 的 OK（串口通了）
 *   板子 LED1 (PF10) 常亮 = AT+CWMODE=1 成功
 *   PE13             常亮 = 连上热点并拿到 IP
 *   板子 LED1 慢闪           = AT 一直没通（查跳线帽/供电/接线）
 *   板子 LED1 快闪           = AT 通了但 WiFi 没连上（会每 10 秒自动重试）
 *
 * 接线要求（P7 座或杜邦线接 UART3 排针都要满足）：
 *   模块 VCC(8) -> 3.3V（不要接 UART3 排针上的 VCC5V，5V 会打死模块）
 *   模块 GND(1) -> GND，模块 TXD(5) -> PB11(RXD3)，模块 RXD(4) -> PB10(TXD3)
 *   模块 EN(6)  -> 3.3V，GPIO0(3)/GPIO2(2) 悬空或上拉
 * ========================================================================== */

#define WIFI_SSID     "YQ-shixun7"    /* 必须是 2.4G 热点，大小写要和热点完全一致 */
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

/* 显示最近一次收到的数据（两行 ASCII + 字节数） */
static void OLED_ShowLastRx(void)
{
    uint8_t  buf[40];
    uint16_t n = ESP8266_Peek(buf, sizeof(buf));

    if (n == 0)
    {
        OLED_ShowString(0, 48, "n=000 (no data)", OLED_6X8);
        OLED_ShowString(0, 56, "                ", OLED_6X8);
        return;
    }
    OLED_ShowAscii(48, buf, (n > 20) ? 20 : n);
    OLED_ShowAscii(56, buf + 20, (n > 20) ? ((n > 40) ? 20 : n - 20) : 0);
}

/* 连接 WiFi，成功返回 1 */
static uint8_t WIFI_Connect(void)
{
    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CWJAP=\"" WIFI_SSID "\",\"" WIFI_PASS "\"");

    /* 连热点很慢，给 15 秒；不同版本固件返回 "WIFI GOT IP" 或直接 "OK" */
    if (ESP8266_WaitResponse("GOT IP", 15000) || ESP8266_WaitResponse("OK", 1000))
        return 1;
    return 0;
}

int main(void)
{
    uint8_t  at_ok = 0;
    uint8_t  i;
    uint8_t  tick = 0;

    LED_init();
    ESP8266_Init();
    OLED_Init();
    OLED_Clear();

    OLED_ShowString(0, 0, "ESP8266 WIFI", OLED_8X16);
    OLED_ShowString(0, 16, "wait boot 1.5s       ", OLED_6X8);
    OLED_ShowString(0, 40, "SSID:" WIFI_SSID, OLED_6X8);
    OLED_Update();

    /* ---------- 1. 等模块启动完成（必须是真实毫秒延时） ---------- */
    DELAY_ms(1500);

    /* ---------- 2. 循环发 AT，直到收到 OK ---------- */
    for (i = 1; i <= AT_RETRY_NUM; i++)
    {
        ESP8266_ClearBuffer();
        ESP8266_SendAT("AT");

        OLED_ShowString(0, 16, "AT try:", OLED_6X8);
        OLED_ShowNum(42, 16, i, 2, OLED_6X8);
        OLED_ShowString(60, 16, "/10", OLED_6X8);
        OLED_Update();

        if (ESP8266_WaitResponse("OK", 1000))
        {
            at_ok = 1;
            break;
        }
    }

    if (at_ok == 0)
    {
        OLED_ShowString(0, 16, "AT FAILED!           ", OLED_6X8);
        OLED_ShowString(0, 24, "check jumper/pwr     ", OLED_6X8);
        OLED_ShowLastRx();
        OLED_Update();
    }
    else
    {
        LED1_on();                                        /* 板子 LED0 (PF9) 亮 = 串口通了 */
        OLED_ShowString(0, 16, "AT OK                ", OLED_6X8);

        /* ---------- 3. 设置 Station 模式 ---------- */
        ESP8266_ClearBuffer();
        ESP8266_SendAT("AT+CWMODE=1");
        if (ESP8266_WaitResponse("OK", 2000))
        {
            LED2_on();                                    /* 板子 LED1 (PF10) 亮 = 模式设置成功 */
            OLED_ShowString(0, 24, "CWMODE OK            ", OLED_6X8);
            OLED_Update();

            /* ---------- 4. 连接 WiFi ---------- */
            OLED_ShowString(0, 32, "connecting...        ", OLED_6X8);
            OLED_Update();

            if (WIFI_Connect())
            {
                wifi_ok = 1;
                LED3_on();                                /* PE13 亮 = 连上并拿到 IP */
                OLED_ShowString(0, 32, "WIFI OK              ", OLED_6X8);

                /* 顺便把 IP 读出来显示（AT+CIFSR 的回复里有 STAIP） */
                ESP8266_ClearBuffer();
                ESP8266_SendAT("AT+CIFSR");
                ESP8266_WaitResponse("OK", 2000);
            }
            else
            {
                OLED_ShowString(0, 32, "WIFI FAILED          ", OLED_6X8);
                OLED_ShowString(0, 40, "SSID must be 2.4G", OLED_6X8);
            }
            OLED_ShowLastRx();
        }
        else
        {
            OLED_ShowString(0, 24, "CWMODE FAILED        ", OLED_6X8);
            OLED_ShowLastRx();
        }
        OLED_Update();
    }

    /* ---------- 5. 主循环：灯语 + 失败自动重试 ---------- */
    while (1)
    {
        if (at_ok == 0)
        {
            /* AT 都没通：板子 LED1 (PF10) 慢闪 */
            LED2_on();
            DELAY_ms(300);
            LED2_off();
            DELAY_ms(300);
        }
        else if (wifi_ok)
        {
            /* 全部成功：三颗灯常亮，什么都不用做 */
            DELAY_ms(500);
        }
        else
        {
            /* AT 通了但 WiFi 没连上：板子 LED1 快闪，每 10 秒重试一次 */
            LED2_on();
            DELAY_ms(150);
            LED2_off();
            DELAY_ms(850);

            tick++;
            if (tick >= 10)
            {
                tick = 0;
                if (WIFI_Connect())
                {
                    wifi_ok = 1;
                    LED2_on();
                    LED3_on();
                    OLED_ShowString(0, 32, "WIFI OK              ", OLED_6X8);
                    OLED_Update();
                }
            }
        }
    }
}
