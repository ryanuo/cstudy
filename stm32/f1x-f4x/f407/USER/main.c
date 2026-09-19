#include "stm32f4xx.h"
#include "OLED.h"
#include "LED.h"
#include "DELAY.h"
#include "esp8266.h"

/* ==========================================================================
 * ESP8266 (AT 固件) 上电测试
 *
 * 两个必须注意的点：
 * 1) 模块上电后要 300ms ~ 1s 才响应 AT。所以上电先"真延时" 1.5s，
 *    然后【循环重发 AT】——只发一次是收不到 OK 的。
 * 2) ESP8266 只支持 2.4G 热点，SSID 不能填 5G 那个。
 *
 * 板子丝印 与 代码函数 的对应（GEC-M4 原理图 02-KEY_LED 页，低电平点亮）：
 *   板子 LED0     = PF9  = LED1_on()   <-- 代码里叫 LED1，别和丝印串了
 *   板子 LED1     = PF10 = LED2_on()   <-- 代码里叫 LED2
 *   板子 FSMC_D10 = PE13 = LED3_on()
 *   板子 FSMC_D11 = PE14 = LED4_on()
 * ========================================================================== */

#define WIFI_SSID     "YQ-SHIXUN2G"   /* 改成实际的 2.4G 热点名 */
#define WIFI_PASS     "88888888"
#define AT_RETRY_NUM  10

/* 一行显示 10 个字节，每个字节占 2 个字符宽 (6x8 字体) */
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

/* 把接收到的原始数据（前 20 字节）+ 长度 + 丢字节数显示出来 */
static void OLED_ShowRxDump(uint8_t *buf, uint16_t len)
{
    OLED_ShowHexLine(24, buf, len > 10 ? 10 : len);
    OLED_ShowHexLine(32, buf + 10, len > 10 ? (len - 10) : 0);

    OLED_ShowString(0, 40, "n=", OLED_6X8);
    OLED_ShowNum(18, 40, len, 3, OLED_6X8);
    OLED_ShowString(54, 40, "lost=", OLED_6X8);
    OLED_ShowNum(90, 40, ESP8266_LostCount(), 3, OLED_6X8);
}

int main(void)
{
    uint8_t  rx[32];
    uint16_t rx_len = 0;
    uint8_t  i;
    uint8_t  at_ok = 0;
    uint8_t  wifi_ok = 0;

    LED_init();
    ESP8266_Init();
    OLED_Init();
    OLED_Clear();

    OLED_ShowString(0, 0, "ESP8266 AT", OLED_8X16);
    OLED_ShowString(0, 16, "wait mini boot", OLED_6X8);
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

        /* 失败就显示收到的原始字节：
           全是 0（n=0）      -> 物理链路问题（TX/RX 没交叉、供电、共地）
           一堆乱码          -> 波特率不对（试 74880 / 9600）
           有内容但没有 OK   -> 命令没发出去或时序问题 */
        rx_len = ESP8266_Peek(rx, sizeof(rx));
        OLED_ShowRxDump(rx, rx_len);
        OLED_ShowString(0, 48, "no OK, retry", OLED_6X8);
        OLED_Update();
    }

    /* ---------- 3. AT 结果 ---------- */
    if (at_ok == 0)
    {
        OLED_ShowString(0, 48, "AT FAILED!", OLED_6X8);
        OLED_ShowString(0, 56, "check wiring", OLED_6X8);
        OLED_Update();
    }
    else
    {
        LED1_on();                                       /* 板子 LED0 (PF9) 亮 = 收到 OK */
        OLED_ShowString(0, 48, "AT OK -> LED0", OLED_6X8);
        OLED_ShowString(0, 56, "CWMODE=1 ...", OLED_6X8);
        OLED_Update();

        /* ---------- 4. 设置为 Station 模式 ---------- */
        ESP8266_ClearBuffer();
        ESP8266_SendAT("AT+CWMODE=1");
        if (ESP8266_WaitResponse("OK", 2000))
        {
            LED2_on();                                   /* 板子 LED1 (PF10) 亮 = 模式设置成功 */
            OLED_ShowString(0, 56, "CWMODE OK", OLED_6X8);
            OLED_Update();

            /* ---------- 5. 连接 WiFi（必须是 2.4G；连接很慢，给 15s） ---------- */
            ESP8266_ClearBuffer();
            ESP8266_SendAT("AT+CWJAP=\"" WIFI_SSID "\",\"" WIFI_PASS "\"");
            if (ESP8266_WaitResponse("GOT IP", 15000) || ESP8266_WaitResponse("OK", 1000))
            {
                wifi_ok = 1;
                LED3_on();                               /* PE13 亮 = 连上热点并拿到 IP */
                OLED_ShowString(0, 56, "WIFI OK", OLED_6X8);
                OLED_Update();
            }
            else
            {
                OLED_ShowString(0, 56, "WIFI FAILED", OLED_6X8);
                rx_len = ESP8266_Peek(rx, sizeof(rx));
                OLED_ShowRxDump(rx, rx_len);
                OLED_Update();
            }
        }
        else
        {
            OLED_ShowString(0, 56, "CWMODE FAILED", OLED_6X8);
            OLED_Update();
        }
    }

    /* ---------- 6. 主循环：用灯把状态一直显示出来，不要静默死等 ---------- */
    while (1)
    {
        if (at_ok == 0)
        {
            /* 连 AT 都没通过：板子 LED1 (PF10) 慢闪 */
            LED2_on();
            DELAY_ms(300);
            LED2_off();
            DELAY_ms(300);
        }
        else if (wifi_ok != 0)
        {
            /* 全部成功：板子 LED0/LED1/PE13 三颗常亮 */
            DELAY_ms(500);
        }
        else
        {
            /* AT 通了但 WiFi 没连上：板子 LED1 (PF10) 快闪提示 */
            LED2_on();
            DELAY_ms(100);
            LED2_off();
            DELAY_ms(900);
        }
    }
}
