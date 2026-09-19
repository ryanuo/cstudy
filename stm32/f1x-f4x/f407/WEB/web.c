#include "web.h"
#include <stdio.h>
#include <string.h>
#include "LED.h"
#include "BEEP.h"
#include "FAN.h"
#include "ADC.h"
#include "LIGHTSENSOR.h"
#include "esp8266.h"

/* ==========================================================================
 * 纯 JSON 接口（不带网页）—— 页面放电脑/云上，跨域调用板子
 *
 * 接口（全部 GET，返回 application/json）：
 *   /                    接口清单
 *   /data                {"led0":0,"led1":1,"led3":1,"led4":0,"fan":0,
 *                         "light":1234,"pot":2048,"req":12}
 *                        （led0/led1/led3/led4/fan 都是直接读引脚回推的真实状态）
 *   /led0/1  /led0/0     板子丝印 LED0（PF9）开/关 -> {"ok":1}
 *   /led1/1  /led1/0     板子丝印 LED1（PF10）开/关 -> {"ok":1}
 *   /led4/1  /led4/0     板子丝印 FSMC_D11（PE14）开/关 -> {"ok":1}
 *   /fan/0  /fan/1  /fan/2   风扇 L9110H（PC6/PC7）：停 / 正转 / 反转 -> {"ok":1}
 *
 * 所有接口都要带 ?k=<WEB_TOKEN>（见 web.h），否则回 {"err":1,"need":"token"}
 *   /beep                蜂鸣器响 200ms        -> {"ok":1}
 *   OPTIONS 任意路径     返回 204 + CORS 头（浏览器跨域预检）
 *
 * 所有响应都带 Access-Control-Allow-Origin: *，所以页面可以从别的源调用
 * （例如电脑上的 http://localhost:5173 或 file://、云上的静态页）。
 *
 * 硬件：光敏 = PF7(ADC3)、电位器 = PA5(ADC1)、蜂鸣器 = PF8、LED0 = PF9、LED1 = PF10
 * ========================================================================== */

#define WEB_CHUNK 2048          /* AT+CIPSEND 单次上限就是 2048 字节 */

static uint16_t req_n = 0;

/* 读真实引脚状态（LED 低电平点亮：输出位为 0 = 亮），不再在软件里记一份状态，
   避免"网页显示的状态"和"板上实际状态"对不上 */
static uint8_t LedOn(GPIO_TypeDef *port, uint16_t pin)
{
    return (GPIO_ReadOutputDataBit(port, pin) == Bit_RESET) ? 1 : 0;
}

/* 风扇状态也靠读引脚回推（电机没有回读，见 FAN_ReadPins）
   0 = 停（PC6/PC7 都低）、1 = 正转（PC6 高）、2 = 反转（PC7 高） */
static uint8_t FanState(void)
{
    uint8_t p = FAN_ReadPins();

    if (p == 0x02) return 1;
    if (p == 0x01) return 2;
    return 0;
}

/* 组装一份完整状态 JSON（/data 和动作接口共用） */
static uint16_t BuildStateJson(char *json)
{
    return (uint16_t)sprintf(json,
        "{\"led0\":%u,\"led1\":%u,\"led3\":%u,\"led4\":%u,\"fan\":%u,"
        "\"light\":%u,\"pot\":%u,\"req\":%u}",
        (unsigned)LedOn(GPIOF, GPIO_Pin_9),    /* 板子丝印 LED0 */
        (unsigned)LedOn(GPIOF, GPIO_Pin_10),   /* 板子丝印 LED1 */
        (unsigned)LedOn(GPIOE, GPIO_Pin_13),   /* 板子丝印 FSMC_D10：服务器指示灯 */
        (unsigned)LedOn(GPIOE, GPIO_Pin_14),   /* 板子丝印 FSMC_D11 */
        (unsigned)FanState(),                  /* 风扇：0 停 / 1 正转 / 2 反转 */
        (unsigned)LIGHT_GetValue(), (unsigned)ADC1ConvertedValue,
        (unsigned)req_n);
}

static void ReplyJson(uint8_t link, const char *body, uint16_t blen);   /* 定义在下面 */

/* 动作接口的回复：{"ok":1, + 最新状态}
   这样页面点一下只发一个请求就能顺便把界面刷新，不用再拉一次 /data */
static void ReplyOkState(uint8_t link)
{
    static char tmp[144];
    static char out[160];

    BuildStateJson(tmp);                        /* {...} */
    sprintf(out, "{\"ok\":1,%s", tmp + 1);      /* 把开头的 '{' 换成 '{"ok":1,' */
    ReplyJson(link, out, (uint16_t)strlen(out));
}

static const char json_err[]  = "{\"err\":1}";
static const char json_deny[] = "{\"err\":1,\"need\":\"token\"}";   /* 令牌不对/没带 */
static const char json_api[] = "{\"api\":\"stm32f407-esp8266\",\"routes\":"
                               "[\"/data\",\"/led0/1\",\"/led0/0\",\"/led1/1\",\"/led1/0\","
                               "\"/led4/1\",\"/led4/0\",\"/fan/0\",\"/fan/1\",\"/fan/2\",\"/beep\"]}";

/* CORS：普通请求只要 ACAO；预检(OPTIONS)还要方法/头 */
#define CORS_HDR "Access-Control-Allow-Origin: *\r\n"
#define CORS_PRE "Access-Control-Allow-Origin: *\r\n" \
                 "Access-Control-Allow-Methods: GET,POST,OPTIONS\r\n" \
                 "Access-Control-Allow-Headers: *\r\n" \
                 "Access-Control-Max-Age: 600\r\n"

/* ---------- 底层：分片发送 + 关闭连接 ---------- */

static void CloseLink(uint8_t link)
{
    char cmd[24];

    sprintf(cmd, "AT+CIPCLOSE=%u", (unsigned)link);
    ESP8266_ClearBuffer();                    /* 清掉上一轮的残留，否则会拿旧的 OK 当回复 */
    ESP8266_SendAT(cmd);
    ESP8266_WaitResponse("OK", 200);          /* 200ms 够；出错时 WaitResponse 会提前返回 */
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
        if (!ESP8266_WaitResponse(">", 2000)) return;    /* 等模块的 '>' 提示符 */

        ESP8266_SendData((uint8_t *)buf, chunk);
        ESP8266_WaitResponse("SEND OK", 5000);

        buf += chunk;
        len -= chunk;
    }
}

/* 回 200 + JSON：头 + 正文拼成一个缓冲一次 CIPSEND 发完
   （原来头发一轮、正文又一轮，每轮都要等 '>' 和 SEND OK，白等一次往返） */
static void ReplyJson(uint8_t link, const char *body, uint16_t blen)
{
    static uint8_t out[512];
    uint16_t hlen;

    hlen = (uint16_t)sprintf((char *)out,
        "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=gbk\r\n"
        CORS_HDR "Cache-Control: no-store\r\n"
        "Content-Length: %u\r\nConnection: close\r\n\r\n", (unsigned)blen);

    if ((uint32_t)hlen + blen > sizeof(out)) blen = (uint16_t)(sizeof(out) - hlen);
    memcpy(out + hlen, body, blen);

    HttpSend(link, (char *)out, (uint16_t)(hlen + blen));
    CloseLink(link);
}

/* 回 204（跨域预检用，没有正文） */
static void ReplyNoContent(uint8_t link)
{
    char     head[200];
    uint16_t hlen;

    hlen = (uint16_t)sprintf(head,
        "HTTP/1.1 204 No Content\r\n" CORS_PRE "Connection: close\r\n\r\n");

    HttpSend(link, head, hlen);
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

static void SendDataJson(uint8_t link)
{
    static char json[144];

    BuildStateJson(json);

    ReplyJson(link, json, (uint16_t)strlen(json));
}

/* 从 "GET /data?k=xxx HTTP/1.1" 里取出路由名 "data"
   注意：必须遇到 '?' 就停：查询串是给令牌用的，算进路由就永远匹配不上（整条路由变成 404） */
static uint8_t PathOf(char *req, char *path, uint8_t max)
{
    char   *g = strstr(req, "GET /");
    uint8_t k = 0, skip = 5;                     /* "GET /" 是 5 个字符 */

    if (g == 0) { g = strstr(req, "POST /"); skip = 6; }   /* "POST /" 是 6 个 —— 别用 5 */
    if (g == 0) { path[0] = '\0'; return 0; }    /* 根本不是 HTTP 请求 */

    g += skip;                                   /* 跳过方法 + 空格 + '/' */
    while (k + 1 < max && *g != '\0' && *g != ' ' && *g != '\r' && *g != '?')
        path[k++] = *g++;
    path[k] = '\0';
    return 1;
}

/* 令牌校验：请求里要有 "?k=<WEB_TOKEN>"（"?k=" 或 "&k=" 开头，避免 ?ak=xx 蒙混）
   简单但够用：这是局域网小工具，防的是"别人扫到 IP 就能开你的灯"，不是防爆破 */
static uint8_t TokenOk(char *req)
{
    char *p = strstr(req, "k=" WEB_TOKEN);
    char *q;

    if (p == 0) return 0;
    if (p != req && p[-1] != '?' && p[-1] != '&') return 0;      /* 挡 ?ak=xxx 这种蒙混 */
    q = p + 2 + (sizeof(WEB_TOKEN) - 1);                         /* 令牌后面的那个字符 */
    return (uint8_t)(*q == '\0' || *q == '&' || *q == ' ' || *q == '\r');
}

/* 处理缓冲里的一个请求：返回 1 = 消费掉了一个请求块，0 = 没有请求了
   关键：用 ESP8266_TakeIp 只消费"这一个"块，别人排队等着的请求不会被清掉 */
static uint8_t HandleOne(void)
{
    static char req[256];
    uint16_t    n;
    uint8_t     link = 0;
    char        path[24];

    n = ESP8266_TakeIp(&link, req, sizeof(req));
    if (n == 0) return 0;

    req_n++;

    /* 跨域预检：浏览器先发 OPTIONS，必须回 204 + CORS 头，否则真实请求不会被发出去。
       这一条不能要令牌：预检的 URL 由浏览器自己拼，带不带参数不受页面控制 */
    if (strstr(req, "OPTIONS") != 0)
    {
        ReplyNoContent(link);
        return 1;
    }

    /* 令牌不对就不执行任何动作，只回一条说明 */
    if (!TokenOk(req))
    {
        ReplyJson(link, json_deny, (uint16_t)(sizeof(json_deny) - 1));
        return 1;
    }

    if (!PathOf(req, path, sizeof(path)))        /* 取路由（已在 '?' 处截断） */
    {
        CloseLink(link);                         /* 不是 HTTP 请求，关掉别占着链接 */
        return 1;
    }

    if (path[0] == '\0')                          ReplyJson(link, json_api, (uint16_t)(sizeof(json_api) - 1));
    else if (strcmp(path, "data") == 0)           SendDataJson(link);
    else if (strcmp(path, "led0/1") == 0)         { LED1_on();  ReplyOkState(link); }  /* 板子 LED0 = PF9 */
    else if (strcmp(path, "led0/0") == 0)         { LED1_off(); ReplyOkState(link); }
    else if (strcmp(path, "led1/1") == 0)         { LED2_on();  ReplyOkState(link); }  /* 板子 LED1 = PF10 */
    else if (strcmp(path, "led1/0") == 0)         { LED2_off(); ReplyOkState(link); }
    else if (strcmp(path, "led4/1") == 0)         { LED4_on();  ReplyOkState(link); }  /* 板子 FSMC_D11 = PE14 */
    else if (strcmp(path, "led4/0") == 0)         { LED4_off(); ReplyOkState(link); }
    else if (strcmp(path, "fan/1") == 0)          { FAN_forwardrotation(); ReplyOkState(link); }  /* 正转 */
    else if (strcmp(path, "fan/2") == 0)          { FAN_reverserotation();  ReplyOkState(link); }  /* 反转 */
    else if (strcmp(path, "fan/0") == 0)          { FAN_off();              ReplyOkState(link); }  /* 停 */
    else if (strcmp(path, "beep")  == 0)          { BEEP_on(); ESP8266_DelayMs(200); BEEP_off(); ReplyOkState(link); }
    else if (strcmp(path, "favicon.ico") == 0)    CloseLink(link);
    else                                          ReplyJson(link, json_err, (uint16_t)(sizeof(json_err) - 1));
    return 1;
}

void Web_Task(void)
{
    uint8_t n = 0;

    /* 一次尽量把排队的请求都处理掉（最多 4 个，免得在这里待太久）。
       以前是处理完一个就 ClearBuffer 清整个缓冲 —— 别人同时发来的请求会被连带清掉，
       而我们的 HTTP 没有重传，那个请求就永久丢了（页面一直转圈） */
    while (n < 4 && HandleOne()) n++;

    if (ESP8266_Find("+IPD,") == 0)              /* 没有积压了才清，AT 回复的垃圾不会越积越多 */
        ESP8266_ClearBuffer();
}

uint16_t Web_ReqCount(void)
{
    return req_n;
}
