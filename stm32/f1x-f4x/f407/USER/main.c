#include "stm32f4xx.h"
#include "OLED.h"
#include "LED.h"
#include "web.h"
#include "esp8266.h"
#include "BEEP.h"
#include "FAN.h"
#include "ADC.h"
#include "LIGHTSENSOR.h"

/* ==========================================================================
 * STM32F407 + ESP8266 (ESP-01S, AT 固件) —— 手机网页控制
 *
 * 手机连同一个路由器，浏览器打开 http://<板子IP>（IP 显示在 OLED 最后一行）。
 * 网页内容和 HTTP 报文都由 web.c 里的代码生成，这个文件只负责：
 *   初始化 -> 联网 -> 开服务器 -> 主循环里调 Web_Task() 处理请求。
 *
 * 灯语：板子 LED0(PF9) 亮 = 串口通了；板子 LED1(PF10) 亮 = WiFi 连上；
 *       PE13 亮 = 服务器已开；板子 LED1 慢闪 = AT 没通；
 *       快闪 = WiFi 断了（每 10 秒重连）
 *
 * 接线：模块 VCC->3.3V（不是排针上的 VCC5V）、GND->GND、
 *       模块 TXD->PB11(RXD3)、模块 RXD->PB10(TXD3)、模块 EN->3.3V，
 *       VCC-GND 间并 100uF。
 * ========================================================================== */

#define WIFI_SSID  "YQ-shixun5"   /* 必须 2.4G，大小写要和热点一致 */
#define WIFI_PASS  "88888888"
#define HTTP_PORT  80

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

/* 风扇引脚自检：在 x,y 处显示 "FAN:10"（两位分别是 PC6、PC7 的输出位）
   不用万用表就能看出"点了正转之后 MCU 到底有没有拉高引脚" */
static void OLED_ShowFanPins(int16_t x, int16_t y)
{
    uint8_t p = FAN_ReadPins();

    OLED_ShowString(x, y, "FAN:", OLED_6X8);
    OLED_ShowNum(x + 24, y, (p >> 1) & 1, 1, OLED_6X8);
    OLED_ShowNum(x + 30, y, p & 1, 1, OLED_6X8);
}

static void OLED_Status(char *ip, uint8_t srv_ok)
{
    OLED_ShowString(0, 24, srv_ok ? "Server:80 OK        " : "Server:80 FAIL      ", OLED_6X8);
    OLED_ShowString(0, 32, "req:", OLED_6X8);
    OLED_ShowNum(24, 32, Web_ReqCount(), 4, OLED_6X8);
    OLED_ShowFanPins(54, 32);                       /* 自检：FAN:xy = PC6,PC7 的实际输出位 */
    OLED_ShowString(0, 40, "SSID:" WIFI_SSID, OLED_6X8);
    OLED_ShowString(0, 48, "http://", OLED_6X8);
    OLED_ShowString(0, 56, (ip != 0 && ip[0] != '\0') ? ip : "no ip", OLED_6X8);
    OLED_Update();
}

int main(void)
{
    char     ip[20];
    uint8_t  wifi_ok = 0, srv_ok = 0, tick = 0, ui = 0;
    uint8_t  rxb[24];
    uint16_t rxn;

    LED_init();
    BEEP_init();                    /* 蜂鸣器 PF8 */
    FAN_init();                     /* 风扇 L9110H: PC6/PC7 */
    ADC1PA5_Init();                 /* 电位器 PA5 / ADC1 + DMA */
    LIGHT_Init();                   /* 光敏 PF7 / ADC3 */
    ESP8266_Init();                 /* USART3 + 1ms 滴答 */
    OLED_Init();
    OLED_Clear();

    OLED_ShowString(0, 0, "ESP8266 WEB", OLED_8X16);
    OLED_ShowString(0, 16, "boot 1.5s           ", OLED_6X8);
    OLED_ShowString(0, 40, "SSID:" WIFI_SSID, OLED_6X8);
    OLED_Update();

    ESP8266_DelayMs(1500);          /* ESP-01S 上电要 300ms~1s 才认 AT */

    /* ---------- 1. 串口 + Station 模式 ---------- */
    if (Web_Init() == 0)
    {
        OLED_ShowString(0, 16, "AT FAILED!          ", OLED_6X8);
        OLED_ShowString(0, 24, "check jumper/pwr    ", OLED_6X8);
        rxn = ESP8266_Peek(rxb, 20);
        OLED_ShowAscii(56, rxb, (rxn > 20) ? 20 : rxn);
        OLED_Update();
        while (1) { LED2_on(); ESP8266_DelayMs(300); LED2_off(); ESP8266_DelayMs(300); }
    }

    LED1_on();                                        /* 板子 LED0 (PF9) 亮 */
    OLED_ShowString(0, 16, "AT OK               ", OLED_6X8);
    OLED_Update();

    /* ---------- 2. 连 WiFi ---------- */
    OLED_ShowString(0, 16, "connecting...       ", OLED_6X8);
    OLED_Update();

    wifi_ok = Web_Connect(WIFI_SSID, WIFI_PASS);
    if (wifi_ok)
    {
        LED2_on();                                    /* 板子 LED1 (PF10) 亮 */
        OLED_ShowString(0, 16, "WiFi: OK            ", OLED_6X8);
    }
    else
    {
        OLED_ShowString(0, 16, "WiFi: FAIL (retry)  ", OLED_6X8);
    }

    /* ---------- 3. 取 IP + 开服务器 ---------- */
    ip[0] = '\0';
    if (wifi_ok)
    {
        Web_GetIp(ip, sizeof(ip));
        srv_ok = Web_OpenServer(HTTP_PORT);
        if (srv_ok) LED3_on();                        /* PE13 亮 */
    }
    OLED_Status(ip, srv_ok);

    /* ---------- 4. 主循环 ---------- */
    while (1)
    {
        if (wifi_ok == 0)
        {
            /* 断线：板子 LED1 快闪，每 10 秒重连一次 */
            LED2_on();
            ESP8266_DelayMs(150);
            LED2_off();
            ESP8266_DelayMs(850);

            if (++tick >= 10)
            {
                tick = 0;
                if (Web_Connect(WIFI_SSID, WIFI_PASS))
                {
                    wifi_ok = 1;
                    LED2_on();
                    OLED_ShowString(0, 16, "WiFi: OK            ", OLED_6X8);
                    Web_GetIp(ip, sizeof(ip));
                    srv_ok = Web_OpenServer(HTTP_PORT);
                    if (srv_ok) LED3_on();
                    OLED_Status(ip, srv_ok);
                }
            }
            continue;
        }

        if (srv_ok == 0)
        {
            /* 连上了但服务器没起来：每 5 秒重开一次 */
            ESP8266_DelayMs(1000);
            if (++tick >= 5)
            {
                tick = 0;
                srv_ok = Web_OpenServer(HTTP_PORT);
                if (srv_ok) LED3_on();
                OLED_Status(ip, srv_ok);
            }
            continue;
        }

        /* 在线：处理网页请求，顺便刷新计数 */
        Web_Task();
        ESP8266_DelayMs(5);          /* 5ms 轮一次：点按钮到出效果更快（原来 50ms）*/

        if (++ui >= 100)             /* 每 500ms 刷一次 OLED */
        {
            ui = 0;
            OLED_ShowString(0, 32, "req:", OLED_6X8);
            OLED_ShowNum(24, 32, Web_ReqCount(), 4, OLED_6X8);
            OLED_ShowFanPins(54, 32);                   /* 自检：FAN:xy = PC6,PC7 */
            OLED_Update();
        }
    }
}
