#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include "OLED.h"
#include "LED.h"
#include "BEEP.h"
#include "ADC.h"
#include "LIGHTSENSOR.h"
#include "esp8266.h"

/* ==========================================================================
 * STM32F407 + ESP8266 (ESP-01S, AT 固件) —— 手机网页控制
 *
 * 手机连同一个路由器，浏览器打开 http://<板子IP> （IP 显示在 OLED 上）：
 *   网页上有 LED0/LED1 开、关、响一下 几个按钮，以及光敏/电位器的实时值。
 *
 * 实现方式：AT 固件开 TCP 服务器（AT+CIPMUX=1 + AT+CIPSERVER=1,80），
 * 收到浏览器发来的 "GET /xxx" 就做动作，再手动拼一个 HTTP 响应发回去。
 *
 * 板子丝印 与 代码函数 的对应（GEC-M4 原理图 02-KEY_LED 页，低电平点亮）：
 *   板子 LED0     = PF9  = LED1_on()/LED1_off()
 *   板子 LED1     = PF10 = LED2_on()/LED2_off()
 *   板子 FSMC_D10 = PE13 = LED3_on()/LED3_off()
 *   蜂鸣器        = PF8  = BEEP_on()/BEEP_off()
 *   光敏 = PF7 (ADC3_IN5, LIGHT_GetValue)、电位器 = PA5 (ADC1_IN5, ADC1ConvertedValue)
 *
 * 接线：模块 VCC->3.3V（不是排针上的 VCC5V）、GND->GND、
 *       模块 TXD->PB11(RXD3)、模块 RXD->PB10(TXD3)、模块 EN->3.3V，
 *       VCC-GND 间并 100uF 抗发射瞬态跌落。
 * ========================================================================== */

#define WIFI_SSID     "YQ-shixun5"   /* 必须是 2.4G 热点，大小写要和热点一致 */
#define WIFI_PASS     "88888888"
#define HTTP_PORT     80
#define AT_RETRY_NUM  10

static uint8_t  led0 = 0, led1 = 0;   /* 网页上显示的 LED 状态 */
static uint16_t req_n = 0;            /* 收到的请求数 */
static char     body[700];            /* HTML 正文 */
static char     page[900];            /* HTTP 头 + 正文 */

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

/* ---------- OLED 状态 ---------- */
static void OLED_Status(char *ip, uint8_t srv_ok)
{
    OLED_ShowString(0, 16, "IP:                 ", OLED_6X8);
    OLED_ShowString(0, 16, "IP:", OLED_6X8);
    OLED_ShowString(18, 16, ip, OLED_6X8);
    OLED_ShowString(0, 24, srv_ok ? "Server:80 OK        " : "Server:80 FAIL      ", OLED_6X8);
    OLED_ShowString(0, 32, "req:", OLED_6X8);
    OLED_ShowNum(24, 32, req_n, 4, OLED_6X8);
    OLED_Update();
}

/* ---------- HTTP ---------- */

/* 拼出 "AT+CIPSEND=<link>,<len>" 并把 resp 原样发出去，然后关连接 */
static void HttpReply(uint8_t link, char *resp, uint16_t len)
{
    char cmd[24];

    sprintf(cmd, "AT+CIPSEND=%u,%u", (unsigned)link, (unsigned)len);
    ESP8266_ClearBuffer();
    ESP8266_SendAT(cmd);
    if (ESP8266_WaitResponse(">", 2000))          /* 模块回 '>' 提示符后再发数据 */
    {
        ESP8266_SendData((uint8_t *)resp, len);   /* 原样发，不加 \r\n */
        ESP8266_WaitResponse("SEND OK", 5000);
    }

    sprintf(cmd, "AT+CIPCLOSE=%u", (unsigned)link);   /* Connection: close */
    ESP8266_SendAT(cmd);
    ESP8266_WaitResponse("OK", 1000);
}

static void CloseLink(uint8_t link)
{
    char cmd[24];

    sprintf(cmd, "AT+CIPCLOSE=%u", (unsigned)link);
    ESP8266_SendAT(cmd);
    ESP8266_WaitResponse("OK", 1000);
}

/* 拼网页（正文 + HTTP 头），返回总长度 */
static uint16_t BuildPage(void)
{
    uint16_t blen, hlen;

    blen = (uint16_t)sprintf(body,
        "<!DOCTYPE html><html><head><meta charset=\"gbk\">"
        "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        "<title>STM32</title></head>"
        "<body style=\"font-family:sans-serif;text-align:center\">"
        "<h3>STM32F407 网页控制</h3>"
        "<p>LED0:%s &nbsp; LED1:%s</p>"
        "<p>光敏:%u &nbsp; 电位器:%u</p>"
        "<p><a href=\"/led0/1\">LED0 开</a> | <a href=\"/led0/0\">LED0 关</a></p>"
        "<p><a href=\"/led1/1\">LED1 开</a> | <a href=\"/led1/0\">LED1 关</a></p>"
        "<p><a href=\"/beep\">响一下</a> | <a href=\"/\">刷新</a></p>"
        "</body></html>",
        led0 ? "ON" : "OFF", led1 ? "ON" : "OFF",
        (unsigned)LIGHT_GetValue(), (unsigned)ADC1ConvertedValue);

    hlen = (uint16_t)sprintf(page,
        "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=gbk\r\n"
        "Cache-Control: no-store\r\n"
        "Content-Length: %u\r\nConnection: close\r\n\r\n", (unsigned)blen);

    memcpy(page + hlen, body, blen);
    return (uint16_t)(hlen + blen);
}

static void SendPage(uint8_t link)
{
    HttpReply(link, page, BuildPage());
}

/* 解析 "GET /xxx"，做动作并回网页 */
static void HandleRequest(void)
{
    char   *p, *g;
    char    path[24];
    uint8_t link, k;

    p = ESP8266_Find("+IPD,");
    if (p == 0) return;                            /* 没有新数据 */

    link = (uint8_t)(p[5] - '0');                  /* "+IPD,<id>,<len>:..." */
    if (link > 4) link = 0;

    g = ESP8266_Find("GET /");
    if (g == 0)
    {
        ESP8266_ClearBuffer();
        CloseLink(link);
        return;
    }

    k = 0;
    g += 5;                                        /* 跳过 "GET /" */
    while (k < 23 && *g != '\0' && *g != ' ' && *g != '\r') path[k++] = *g++;
    path[k] = '\0';

    ESP8266_ClearBuffer();                         /* 解析完了，清掉缓冲等下一个请求 */
    req_n++;

    if (path[0] == '\0')
        SendPage(link);                            /* 打开的是首页 */
    else if (strcmp(path, "led0/1") == 0) { LED1_on();  led0 = 1; SendPage(link); }  /* 板子 LED0 = PF9 */
    else if (strcmp(path, "led0/0") == 0) { LED1_off(); led0 = 0; SendPage(link); }
    else if (strcmp(path, "led1/1") == 0) { LED2_on();  led1 = 1; SendPage(link); }  /* 板子 LED1 = PF10 */
    else if (strcmp(path, "led1/0") == 0) { LED2_off(); led1 = 0; SendPage(link); }
    else if (strcmp(path, "beep")  == 0)  { BEEP_on(); ESP8266_DelayMs(200); BEEP_off(); SendPage(link); }
    else if (strcmp(path, "favicon.ico") == 0) CloseLink(link);
    else SendPage(link);                           /* 其他路径一律回首页 */
}

/* ---------- 联网 ---------- */

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

/* 开 TCP 服务器（多连接模式 + 端口 80） */
static uint8_t OpenServer(void)
{
    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CIPMUX=1");
    if (!ESP8266_WaitResponse("OK", 2000)) return 0;

    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CIPSERVER=0");              /* 先关掉可能还开着的服务 */
    ESP8266_WaitResponse("OK", 1000);

    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CIPSERVER=1,80");
    return ESP8266_WaitResponse("OK", 3000);
}

int main(void)
{
    uint8_t  at_ok = 0, wifi_ok = 0, srv_ok = 0;
    uint8_t  i, tick = 0, ui = 0;
    char     ip[20];
    uint8_t  rxb[24];
    uint16_t rxn;

    LED_init();
    ESP8266_Init();                 /* USART3 + 1ms 滴答 */
    BEEP_init();
    LIGHT_Init();                   /* 光敏 PF7 / ADC3 */
    ADC1PA5_Init();                 /* 电位器 PA5 / ADC1 + DMA */
    OLED_Init();
    OLED_Clear();

    OLED_ShowString(0, 0, "ESP8266 WEB", OLED_8X16);
    OLED_ShowString(0, 16, "boot 1.5s           ", OLED_6X8);
    OLED_ShowString(0, 40, "SSID:" WIFI_SSID, OLED_6X8);
    OLED_Update();

    ESP8266_DelayMs(1500);          /* ESP-01S 上电要 300ms~1s 才认 AT */

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
        OLED_ShowAscii(56, rxb, (rxn > 20) ? 20 : rxn);   /* 显示模块原话 */
        OLED_Update();
        while (1) { LED2_on(); ESP8266_DelayMs(300); LED2_off(); ESP8266_DelayMs(300); }
    }

    LED1_on();                                        /* 板子 LED0 (PF9) 亮 = AT 通了 */
    OLED_ShowString(0, 16, "AT OK               ", OLED_6X8);
    OLED_Update();

    /* ---------- 2. Station 模式 + 连 WiFi ---------- */
    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CWMODE=1");
    if (ESP8266_WaitResponse("OK", 2000)) LED2_on();

    OLED_ShowString(0, 24, "connecting...       ", OLED_6X8);
    OLED_Update();

    if (WIFI_Connect())
    {
        wifi_ok = 1;
        LED3_on();
        OLED_ShowString(0, 24, "WIFI OK             ", OLED_6X8);
    }
    else
    {
        OLED_ShowString(0, 24, "WIFI FAIL, retry    ", OLED_6X8);
    }

    /* ---------- 3. 取 IP + 开服务器 ---------- */
    if (wifi_ok)
    {
        for (i = 0; i < 3; i++)                       /* 刚连上模块可能还忙，重试几次 */
        {
            ESP8266_ClearBuffer();
            ESP8266_SendAT("AT+CIFSR");
            ESP8266_WaitResponse("OK", 3000);
            if (ESP8266_FindIp(ip, sizeof(ip))) break;
            ESP8266_DelayMs(500);
        }
        if (i == 3) ip[0] = '\0';

        srv_ok = OpenServer();
        OLED_Status(ip[0] ? ip : "no ip", srv_ok);
    }
    OLED_Update();

    /* ---------- 4. 主循环 ---------- */
    while (1)
    {
        if (wifi_ok == 0)
        {
            /* 没连上：板子 LED1 快闪，每 10 秒重连一次 */
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
                    OLED_ShowString(0, 24, "WIFI OK             ", OLED_6X8);
                    ESP8266_ClearBuffer();
                    ESP8266_SendAT("AT+CIFSR");
                    ESP8266_WaitResponse("OK", 3000);
                    if (!ESP8266_FindIp(ip, sizeof(ip))) ip[0] = '\0';
                    srv_ok = OpenServer();
                    OLED_Status(ip[0] ? ip : "no ip", srv_ok);
                }
            }
            continue;
        }

        if (srv_ok == 0)
        {
            /* 连上了但服务器没起来：每 5 秒重试开一次 */
            ESP8266_DelayMs(1000);
            if (++tick >= 5)
            {
                tick = 0;
                srv_ok = OpenServer();
                if (srv_ok) LED3_on();
                OLED_Status(ip[0] ? ip : "no ip", srv_ok);
            }
            continue;
        }

        /* 在线且服务器已开：处理网页请求，顺便刷新计数 */
        HandleRequest();
        ESP8266_DelayMs(50);

        if (++ui >= 10)
        {
            ui = 0;
            OLED_ShowString(0, 32, "req:", OLED_6X8);
            OLED_ShowNum(24, 32, req_n, 4, OLED_6X8);
            OLED_ShowString(0, 40, "open on phone:", OLED_6X8);
            OLED_ShowString(0, 48, "http://", OLED_6X8);
            OLED_ShowString(0, 56, ip, OLED_6X8);
            OLED_Update();
        }
    }
}
