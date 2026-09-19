#include "web.h"
#include <stdio.h>
#include <string.h>
#include "LED.h"
#include "BEEP.h"
#include "ADC.h"
#include "LIGHTSENSOR.h"
#include "esp8266.h"

/* ==========================================================================
 * 网页控制（板子当 Web 服务器）
 *
 * 分工：网页内容和 HTTP 报文都由 STM32 拼（这个文件），ESP8266 只负责把字节
 *       从串口搬到 TCP 上。页面本体放在 Flash（const），不占 RAM。
 *
 * 板子丝印 与 代码函数（GEC-M4 原理图 02-KEY_LED 页，低电平点亮）：
 *   板子 LED0 = PF9  = LED1_on()/LED1_off()
 *   板子 LED1 = PF10 = LED2_on()/LED2_off()
 *   蜂鸣器    = PF8  = BEEP_on()/BEEP_off()
 *   光敏 = PF7 (ADC3/LIGHT_GetValue)、电位器 = PA5 (ADC1/ADC1ConvertedValue)
 * ========================================================================== */

#define WEB_CHUNK 2048          /* AT+CIPSEND 单次上限就是 2048 字节 */

static uint8_t  led0 = 0, led1 = 0;   /* 网页上显示的 LED 状态 */
static uint16_t req_n = 0;

/* 网页本体：Vue 3 由手机从 CDN 下载运行，板子只发这一段 HTML。
   样式是手写的 shadcn 风格（zinc 灰阶 + 12px 圆角 + 细边框 + filled 徽章），
   真·shadcn 需要 Tailwind 构建，CDN 路线用不了。 */
static const char page_html[] =
"<!DOCTYPE html><html lang=\"zh\"><head><meta charset=\"gbk\">"
"<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
"<title>STM32F407 控制台</title>"
"<script src=\"https://unpkg.com/vue@3/dist/vue.global.prod.js\"></script>"
"<style>"
"*{box-sizing:border-box}"
"body{margin:0;padding:16px;background:#f4f4f5;color:#18181b;"
"font:15px/1.6 system-ui,-apple-system,sans-serif}"
".c{max-width:420px;margin:0 auto;background:#fff;border:1px solid #e4e4e7;"
"border-radius:12px;padding:16px}"
"h2{margin:0 0 12px;font-size:16px;font-weight:600}"
".r{display:flex;align-items:center;gap:8px;padding:10px 0;border-top:1px solid #f4f4f5}"
".r>span:first-child{width:64px;color:#71717a;font-size:14px}"
".b{margin-left:auto;font-size:12px;padding:2px 8px;border-radius:6px;"
"background:#f4f4f5;color:#71717a}"
".on{background:#18181b;color:#fafafa}"
"button{padding:5px 12px;font-size:13px;border:1px solid #e4e4e7;border-radius:8px;"
"background:#fff;color:#18181b}"
"button:active{background:#f4f4f5}"
".f{margin-top:12px;color:#a1a1aa;font-size:12px;text-align:center}"
"</style></head><body>"
"<div id=\"app\"><div class=\"c\">"
"<h2>STM32F407 控制台</h2>"
"<div class=\"r\"><span>LED0</span>"
"<span class=\"b\" :class=\"{on:s.led0}\">{{s.led0?'ON':'OFF'}}</span>"
"<button @click=\"c('led0/1')\">开</button>"
"<button @click=\"c('led0/0')\">关</button></div>"
"<div class=\"r\"><span>LED1</span>"
"<span class=\"b\" :class=\"{on:s.led1}\">{{s.led1?'ON':'OFF'}}</span>"
"<button @click=\"c('led1/1')\">开</button>"
"<button @click=\"c('led1/0')\">关</button></div>"
"<div class=\"r\"><span>蜂鸣器</span>"
"<button style=\"margin-left:auto\" @click=\"c('beep')\">响一下</button></div>"
"<div class=\"r\"><span>光敏</span><b>{{s.light}}</b></div>"
"<div class=\"r\"><span>电位器</span><b>{{s.pot}}</b></div>"
"<div class=\"r\"><span>请求数</span><b>{{s.req}}</b></div>"
"<div class=\"f\">Vue 3 由 CDN 加载 · 每秒自动刷新</div>"
"</div></div>"
"<script>"
"const{createApp,ref,onMounted}=Vue;"
"createApp({setup(){const s=ref({});"
"const l=()=>fetch('/data').then(r=>r.json()).then(j=>s.value=j);"
"const c=p=>fetch('/'+p).then(l);"
"onMounted(()=>{l();setInterval(l,1000)});"
"return{s,c}}}).mount('#app');"
"</script></body></html>";

static const char json_ok[] = "{\"ok\":1}";

/* ---------- 底层：分片发送 + 关闭连接 ---------- */

static void CloseLink(uint8_t link)
{
    char cmd[24];

    sprintf(cmd, "AT+CIPCLOSE=%u", (unsigned)link);
    ESP8266_SendAT(cmd);
    ESP8266_WaitResponse("OK", 1000);
}

/* 按 2048 字节分片发（AT+CIPSEND 单次上限），每片等 SEND OK 再发下一片 */
static void HttpSend(uint8_t link, const char *buf, uint16_t len)
{
    char     cmd[24];
    uint16_t chunk;

    while (len > 0)
    {
        chunk = (len > WEB_CHUNK) ? WEB_CHUNK : len;

        sprintf(cmd, "AT+CIPSEND=%u,%u", (unsigned)link, (unsigned)chunk);
        ESP8266_ClearBuffer();
        ESP8266_SendAT(cmd);
        if (!ESP8266_WaitResponse(">", 2000)) return;   /* 等模块的 '>' 提示符 */

        ESP8266_SendData((uint8_t *)buf, chunk);
        ESP8266_WaitResponse("SEND OK", 5000);

        buf += chunk;
        len -= chunk;
    }
}

/* 发一个完整响应：先发头，再发正文（TCP 是字节流，浏览器拼得起来） */
static void Reply(uint8_t link, const char *ctype, const char *body, uint16_t blen)
{
    char     head[160];
    uint16_t hlen;

    hlen = (uint16_t)sprintf(head,
        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nCache-Control: no-store\r\n"
        "Content-Length: %u\r\nConnection: close\r\n\r\n",
        ctype, (unsigned)blen);

    HttpSend(link, head, hlen);
    HttpSend(link, body, blen);
    CloseLink(link);
}

/* ---------- 联网 ---------- */

uint8_t Web_Init(void)
{
    uint8_t i;

    for (i = 0; i < 10; i++)                     /* ESP-01S 上电要 300ms~1s 才认 AT */
    {
        ESP8266_ClearBuffer();
        ESP8266_SendAT("AT");
        if (ESP8266_WaitResponse("OK", 1000)) break;
    }
    if (i >= 10) return 0;

    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CWMODE=1");
    return ESP8266_WaitResponse("OK", 2000);
}

uint8_t Web_Connect(char *ssid, char *pass)
{
    char     cmd[96];
    uint32_t start;

    strcpy(cmd, "AT+CWJAP=\"");
    strcat(cmd, ssid);
    strcat(cmd, "\",\"");
    strcat(cmd, pass);
    strcat(cmd, "\"");

    ESP8266_ClearBuffer();
    ESP8266_SendAT(cmd);

    start = ESP8266_GetTick();
    while ((uint32_t)(ESP8266_GetTick() - start) < 15000)
    {
        if (ESP8266_Contains("GOT IP") || ESP8266_Contains("OK"))    return 1;
        if (ESP8266_Contains("FAIL")   || ESP8266_Contains("ERROR")) return 0;
    }
    return 0;
}

uint8_t Web_GetIp(char *ip, uint8_t max_len)
{
    uint8_t i;

    for (i = 0; i < 3; i++)                      /* 刚连上模块可能还忙，重试几次 */
    {
        ESP8266_ClearBuffer();
        ESP8266_SendAT("AT+CIFSR");
        ESP8266_WaitResponse("OK", 3000);
        if (ESP8266_FindIp(ip, max_len)) return 1;
        ESP8266_DelayMs(500);
    }
    ip[0] = '\0';
    return 0;
}

uint8_t Web_OpenServer(uint16_t port)
{
    char cmd[24];

    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CIPMUX=1");                       /* 多连接，服务器模式必需 */
    if (!ESP8266_WaitResponse("OK", 2000)) return 0;

    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CIPSERVER=0");                    /* 先关掉可能还开着的服务 */
    ESP8266_WaitResponse("OK", 1000);

    sprintf(cmd, "AT+CIPSERVER=1,%u", (unsigned)port);
    ESP8266_ClearBuffer();
    ESP8266_SendAT(cmd);
    return ESP8266_WaitResponse("OK", 3000);
}

/* ---------- 请求处理 ---------- */

static void SendPage(uint8_t link)
{
    Reply(link, "text/html; charset=gbk", page_html, (uint16_t)(sizeof(page_html) - 1));
}

static void SendDataJson(uint8_t link)
{
    char json[96];

    sprintf(json, "{\"led0\":%u,\"led1\":%u,\"light\":%u,\"pot\":%u,\"req\":%u}",
            (unsigned)led0, (unsigned)led1,
            (unsigned)LIGHT_GetValue(), (unsigned)ADC1ConvertedValue,
            (unsigned)req_n);

    Reply(link, "application/json", json, (uint16_t)strlen(json));
}

void Web_Task(void)
{
    char    *p, *g;
    char     path[24];
    uint8_t  link, k;

    p = ESP8266_Find("+IPD,");                   /* 有数据进来 */
    if (p == 0) return;

    link = (uint8_t)(p[5] - '0');                /* "+IPD,<id>,<len>:..." */
    if (link > 4) link = 0;

    g = ESP8266_Find("GET /");                   /* 浏览器的请求行 */
    if (g == 0)
    {
        ESP8266_ClearBuffer();
        CloseLink(link);
        return;
    }

    k = 0;
    g += 5;                                      /* 跳过 "GET /" */
    while (k < 23 && *g != '\0' && *g != ' ' && *g != '\r') path[k++] = *g++;
    path[k] = '\0';

    ESP8266_ClearBuffer();                       /* 解析完了就清缓冲，等下一个请求 */
    req_n++;

    if (path[0] == '\0')                          SendPage(link);
    else if (strcmp(path, "data") == 0)           SendDataJson(link);
    else if (strcmp(path, "led0/1") == 0)         { LED1_on();  led0 = 1; Reply(link, "application/json", json_ok, 8); }  /* 板子 LED0 = PF9 */
    else if (strcmp(path, "led0/0") == 0)         { LED1_off(); led0 = 0; Reply(link, "application/json", json_ok, 8); }
    else if (strcmp(path, "led1/1") == 0)         { LED2_on();  led1 = 1; Reply(link, "application/json", json_ok, 8); }  /* 板子 LED1 = PF10 */
    else if (strcmp(path, "led1/0") == 0)         { LED2_off(); led1 = 0; Reply(link, "application/json", json_ok, 8); }
    else if (strcmp(path, "beep")  == 0)          { BEEP_on(); ESP8266_DelayMs(200); BEEP_off(); Reply(link, "application/json", json_ok, 8); }
    else if (strcmp(path, "favicon.ico") == 0)    CloseLink(link);
    else                                          Reply(link, "application/json", "{\"err\":1}", 9);
}

uint16_t Web_ReqCount(void)
{
    return req_n;
}
