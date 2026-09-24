#include "stm32f4xx.h"
#include "OLED.h"
#include "LED.h"
#include "web.h"
#include "esp8266.h"
#include "BEEP.h"
#include "FAN.h"
#include "ADC.h"
#include "LIGHTSENSOR.h"
#include "DHT11.h"
#include "SPI.h"
#include "CAN.h"

/* ==========================================================================
 * STM32F407 + ESP8266 (ESP-01S, AT ???) ???? ??????????
 *
 * ?????????·????????????? http://<????IP>??IP ????? OLED ?????У???
 * ???????? HTTP ??????? web.c ??????????????????????
 *   ????? -> ???? -> ???????? -> ???????? Web_Task() ????????
 *
 * ???????? LED1(PF9) ?? = ???????????? LED2(PF10)  = WiFi ?
 *       PE13  =? LED3 = AT ??
 *       = WiFi ?? 10
 *
 * ?? VCC->3.3V? VCC5VGND->GND
 *       ? TXD->PB11(RXD3)? RXD->PB10(TXD3)? EN->3.3V
 *       VCC-GND</tool_call> 100uF
 * ========================================================================== */

// #define WIFI_SSID  "Magic7"   /* ???? 2.4G????Сд????????? */
// #define WIFI_PASS  "121qweqwe"
#define WIFI_SSID "YQ-shixun5" /* ???? 2.4G????Сд????????? */
#define WIFI_PASS "88888888"
#define HTTP_PORT 80

/* ?????????????? ASCII ?????????????????? .??????? 20 ???? */
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

/* ????????????? x,y ????? "FAN:10"????λ????? PC6??PC7 ?????λ??
   ????????????????"?????????? MCU ?????????????????" */
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
    OLED_ShowFanPins(54, 32); /* ???FAN:xy = PC6,PC7 ????????λ */
    OLED_ShowString(0, 40, "SSID:" WIFI_SSID, OLED_6X8);
    OLED_ShowString(0, 48, "http://", OLED_6X8);
    OLED_ShowString(0, 56, (ip != 0 && ip[0] != '\0') ? ip : "no ip", OLED_6X8);
    OLED_Update();
}

void LED_Status()
{
    LED4_on();
    LED1_off();
    LED2_off();
    LED3_off();
}

int main(void)
{
    char ip[20];
    uint8_t wifi_ok = 0, srv_ok = 0, tick = 0, ui = 0;
    uint8_t rxb[24];
    uint16_t rxn;
    uint16_t last_req = 0;      /* 自愈用：上次看到的请求计数 */
    uint32_t last_req_tick = 0; /* 自愈用：上次收到请求的时刻 */

    LED_init();
    LED_FlowInit();
    BEEP_init();    /* ?????? PF8 */
    FAN_init();     /* ???? L9110H: PC6/PC7 */
    ADC1PA5_Init(); /* ??λ?? PA5 / ADC1 + DMA */
    LIGHT_Init();   /* ???? PF7 / ADC3 */
    DHT11_Init();   /* ????? DHT11: PG9 (???? U6 ??) */
    ESP8266_Init(); /* USART3 + 1ms ?δ? */
    SPI1_init();    /* SPI1 + GPIOB 3/4/5/14 */
    CAN_init();     /* CAN1
    
    /* ---------- 1. ???? + Station ?? ---------- */
    if (Web_Init() == 0)
    {
        OLED_ShowString(0, 16, "AT FAILED!          ", OLED_6X8);
        OLED_ShowString(0, 24, "check jumper/pwr    ", OLED_6X8);
        rxn = ESP8266_Peek(rxb, 20);
        OLED_ShowAscii(56, rxb, (rxn > 20) ? 20 : rxn);
        OLED_Update();
        while (1)
        {
            LED2_on();
            ESP8266_DelayMs(300);
            LED2_off();
            ESP8266_DelayMs(300);
        }
    }

    LED1_on(); /* ???? LED1 (PF9)  */
    OLED_ShowString(0, 16, "AT OK               ", OLED_6X8);
    OLED_Update();

    /* ---------- 2. ?? WiFi ---------- */
    OLED_ShowString(0, 16, "connecting...       ", OLED_6X8);
    OLED_Update();

    wifi_ok = Web_Connect(WIFI_SSID, WIFI_PASS);
    if (wifi_ok)
    {
        LED2_on(); /* ???? LED1 (PF10) ?? */
        OLED_ShowString(0, 16, "WiFi: OK            ", OLED_6X8);
    }
    else
    {
        OLED_ShowString(0, 16, "WiFi: FAIL (retry)  ", OLED_6X8);
    }

    /* ---------- 3. ? IP + ???????? ---------- */
    ip[0] = '\0';
    if (wifi_ok)
    {
        Web_GetIp(ip, sizeof(ip));
        srv_ok = Web_OpenServer(HTTP_PORT);
        if (srv_ok)
            LED_Status(); /* PE13 ?? */
    }
    OLED_Status(ip, srv_ok);
    last_req_tick = ESP8266_GetTick();

    /* ---------- 4. ????? ---------- */
    while (1)
    {
        if (wifi_ok == 0)
        {
            /* ????????? LED1 ??????? 10 ????????? */
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
                    if (srv_ok)
                    {
                        LED_Status();
                    }
                    OLED_Status(ip, srv_ok);
                }
            }
            continue;
        }

        if (srv_ok == 0)
        {
            /* ????????????????????? 5 ???????? */
            ESP8266_DelayMs(1000);
            if (++tick >= 5)
            {
                tick = 0;
                srv_ok = Web_OpenServer(HTTP_PORT);
                if (srv_ok)
                {
                    LED_Status();
                }
                OLED_Status(ip, srv_ok);
            }
            continue;
        }

        /* ????????????????????????? */
        DHT11_Task(); /* ? 2 ?????????????????? 25ms??*/
        Web_Task();

        /* 静默自愈：连续 30 秒没有任何请求，就清掉所有残链接并重开服务器。
           出现过"回复期间请求被丢 -> 那些连接没人关 -> 5 个槽位占满 -> 服务器不再 accept"，
           那时 srv_ok 一直是 1、永远不会重开服务器，只能复位；有了这段就能自己爬起来。 */
        if (Web_ReqCount() != last_req)
        {
            last_req = Web_ReqCount();
            last_req_tick = ESP8266_GetTick();
        }
        else if ((uint32_t)(ESP8266_GetTick() - last_req_tick) >= 30000U)
        {
            last_req_tick = ESP8266_GetTick();
            srv_ok = Web_ResetServer(HTTP_PORT);
            OLED_Status(ip, srv_ok);
        }

        BEEP_Task();
        LED_FlowRun();
        ESP8266_DelayMs(5); /* 5ms ????Σ????????Ч????????? 50ms??*/

        if (++ui >= 100) /* ? 500ms ???? OLED */
        {
            ui = 0;
            OLED_ShowString(0, 32, "req:", OLED_6X8);
            OLED_ShowNum(24, 32, Web_ReqCount(), 4, OLED_6X8);
            OLED_ShowFanPins(54, 32); /* ???FAN:xy = PC6,PC7 */
            OLED_Update();
        }
    }
}
