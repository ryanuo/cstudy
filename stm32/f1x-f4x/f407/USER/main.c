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
 *   板子 LED0 (PF9)  常亮 = 收到 AT 的 OK（串口通了）
 *   板子 LED1 (PF10) 常亮 = AT+CWMODE=1 成功
 *   PE13             常亮 = 连上热点并拿到 IP
 *   板子 LED1 慢闪           = AT 一直没通（查跳线帽/供电/接线）
 *   板子 LED1 快闪           = AT 通了但 WiFi 没连上（每 10 秒自动重试）
 *
 * 接线要求：模块 VCC->3.3V（不是排针上的 VCC5V！）、GND->GND、
 *           模块 TXD->PB11(RXD3)、模块 RXD->PB10(TXD3)、模块 EN->3.3V，
 *           GPIO0/GPIO2 悬空或上拉；VCC-GND 间并 100uF 抗发射瞬态跌落。
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

/* 显示最近一次收到的数据（两行 ASCII） */
static void OLED_ShowLastRx(void)
{
    uint8_t  buf[40];
    uint16_t n = ESP8266_Peek(buf, sizeof(buf));

    if (n == 0)
    {
        OLED_ShowString(0, 48, "n=000 (no data)     ", OLED_6X8);
        OLED_ShowString(0, 56, "                    ", OLED_6X8);
        return;
    }
    OLED_ShowAscii(48, buf, (n > 20) ? 20 : n);
    OLED_ShowAscii(56, buf + 20, (n > 20) ? ((n > 40) ? 20 : n - 20) : 0);
}

/* 连 WiFi：15 秒内等到 "GOT IP" 或 "OK" 算成功；
   出现 FAIL / ERROR 立刻返回，不用干等 15 秒。
   超时/失败后用 OLED_ShowLastRx() 就能看到模块的原话。 */
static uint8_t WIFI_Connect(void)
{
    uint32_t start;

    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CWJAP=\"" WIFI_SSID "\",\"" WIFI_PASS "\"");

    start = ESP8266_GetTick();
    while ((uint32_t)(ESP8266_GetTick() - start) < 15000)
    {
        if (ESP8266_Contains("GOT IP") || ESP8266_Contains("OK"))  return 1;
        if (ESP8266_Contains("FAIL")   || ESP8266_Contains("ERROR")) return 0;
    }
    return 0;
}

int main(void)
{
    uint8_t  at_ok = 0;
    uint8_t  i;
    uint8_t  tick = 0;

    LED_init();
    ESP8266_Init();          /* 里面会把 USART3 和 1ms 滴答都起好 */
    OLED_Init();
    OLED_Clear();

    OLED_ShowString(0, 0, "ESP8266 WIFI", OLED_8X16);
    OLED_ShowString(0, 16, "wait boot 1.5s       ", OLED_6X8);
    OLED_ShowString(0, 40, "SSID:" WIFI_SSID, OLED_6X8);
    OLED_Update();

    /* ---------- 1. 等模块启动完成（ESP-01S 上电要 300ms~1s 才认 AT） ---------- */
    ESP8266_DelayMs(1500);

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
                ESP8266_WaitResponse("OK", 3000);
            }
            else
            {
                OLED_ShowString(0, 32, "WIFI FAILED          ", OLED_6X8);
                OLED_ShowString(0, 40, "SSID must be 2.4G    ", OLED_6X8);
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
            ESP8266_DelayMs(300);
            LED2_off();
            ESP8266_DelayMs(300);
        }
        else if (wifi_ok)
        {
            /* 全部成功：三颗灯常亮，什么都不用做 */
            ESP8266_DelayMs(500);
        }
        else
        {
            /* AT 通了但 WiFi 没连上：板子 LED1 快闪，每 10 秒重试一次 */
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
                    OLED_ShowString(0, 32, "WIFI OK              ", OLED_6X8);
                    OLED_ShowLastRx();
                    OLED_Update();
                }
            }
        }
    }
}
