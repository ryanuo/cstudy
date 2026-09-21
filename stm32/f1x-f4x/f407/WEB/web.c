#include "web.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "esp8266.h"

#define WEB_CHUNK 2048

static uint16_t req_n = 0;

/* ---------------- 请求解析 ---------------- */

uint8_t Web_PathOf(char *req, char *path, uint8_t max)
{
    char   *g = strstr(req, "GET /");
    uint8_t k = 0, skip = 5;

    if (g == 0) { g = strstr(req, "POST /"); skip = 6; }
    if (g == 0) { path[0] = '\0'; return 0; }

    g += skip;
    while (k + 1 < max && *g != '\0' && *g != ' ' && *g != '\r' && *g != '?')
        path[k++] = *g++;
    path[k] = '\0';
    return 1;
}

uint8_t Web_TokenOk(char *req)
{
    char *p = strstr(req, "k=" WEB_TOKEN);
    char *q;

    if (p == 0) return 0;
    if (p != req && p[-1] != '?' && p[-1] != '&') return 0;
    q = p + 2 + (sizeof(WEB_TOKEN) - 1);
    return (uint8_t)(*q == '\0' || *q == '&' || *q == ' ' || *q == '\r');
}

uint32_t Web_GetParamU32(char *req, const char *key, uint32_t def)
{
    char     pat[16];
    char    *p;
    uint32_t v = 0;
    uint8_t  base = 10;

    sprintf(pat, "%s=", key);
    p = strstr(req, pat);
    if (p == 0) return def;
    if (p != req && p[-1] != '?' && p[-1] != '&' && p[-1] != ' ') return def;

    p += strlen(pat);
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) { base = 16; p += 2; }

    while (*p) {
        uint8_t c = (uint8_t)*p, d;
        if      (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
        else break;
        if (d >= base) break;
        v = v * base + d;
        p++;
    }
    return v;
}

/* ---------------- 底层：分片发送 / 关闭 ---------------- */

void Web_CloseLink(uint8_t link)
{
    char cmd[24];

    sprintf(cmd, "AT+CIPCLOSE=%u", (unsigned)link);
    /* 用 ClearNonIp 而不是 ClearBuffer：这里离下一个请求往往只差几毫秒，
       整块清空会把刚排队进来的请求一起丢掉（就是"第一次能用、之后没反应"的那个坑） */
    ESP8266_ClearNonIp();
    ESP8266_SendAT(cmd);
    ESP8266_WaitResponse("OK", 200);
}

static void HttpSend(uint8_t link, const char *buf, uint16_t len)
{
    char     cmd[24];
    uint16_t chunk;

    while (len > 0) {
        chunk = (len > WEB_CHUNK) ? WEB_CHUNK : len;

        sprintf(cmd, "AT+CIPSEND=%u,%u", (unsigned)link, (unsigned)chunk);
        ESP8266_ClearNonIp();      /* 同上：清 AT 噪声但留住排队中的请求 */
        ESP8266_SendAT(cmd);
        if (!ESP8266_WaitResponse(">", 2000)) return;

        ESP8266_SendData((uint8_t *)buf, chunk);
        ESP8266_WaitResponse("SEND OK", 5000);

        buf += chunk;
        len -= chunk;
    }
}

/* ---------------- 回复 ---------------- */

void Web_ReplyJson(uint8_t link, const char *body, uint16_t blen)
{
    static uint8_t out[1024];      /* 512 -> 1024，给 Flash hex 留空间 */
    uint16_t hlen;

    hlen = (uint16_t)sprintf((char *)out,
        "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=gbk\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Cache-Control: no-store\r\n"
        "Content-Length: %u\r\nConnection: close\r\n\r\n", (unsigned)blen);

    if ((uint32_t)hlen + blen > sizeof(out)) blen = (uint16_t)(sizeof(out) - hlen);
    memcpy(out + hlen, body, blen);

    HttpSend(link, (char *)out, (uint16_t)(hlen + blen));
    Web_CloseLink(link);
}

void Web_ReplyOk(uint8_t link)
{
    Web_ReplyJson(link, "{\"ok\":1}", 8);
}

void Web_ReplyErr(uint8_t link)
{
    Web_ReplyJson(link, "{\"err\":1}", 9);
}

/* ---------------- /data 聚合 ---------------- */

static char     state_buf[512];
static uint16_t state_off;

void Web_StateReset(void)
{
    state_off = 0;
    state_buf[state_off++] = '{';
    state_buf[state_off]   = '\0';
}

void Web_StateAppend(const char *fmt, ...)
{
    va_list ap;
    int     n;

    if (state_off > 1) {
        state_buf[state_off++] = ',';
        state_buf[state_off]   = '\0';
    }

    va_start(ap, fmt);
    n = vsnprintf(state_buf + state_off, sizeof(state_buf) - state_off, fmt, ap);
    va_end(ap);

    if (n > 0) state_off += (uint16_t)n;
}

void Web_StateSend(uint8_t link)
{
    if (state_off < sizeof(state_buf) - 1) {
        state_buf[state_off++] = '}';
        state_buf[state_off]   = '\0';
    }
    Web_ReplyJson(link, state_buf, state_off);
}

void Web_SendAllState(uint8_t link)
{
    Web_StateReset();
    Led_WebState();
    Fan_WebState();
    Sensor_WebState();
    Web_StateAppend("\"req\":%u", (unsigned)req_n);
    Web_StateSend(link);
}

void Web_ReplyOkState(uint8_t link)
{
    static char tmp[512];

    Web_StateReset();
    Led_WebState();
    Fan_WebState();
    Sensor_WebState();
    Web_StateAppend("\"req\":%u", (unsigned)req_n);
    if (state_off < sizeof(state_buf) - 1) {
        state_buf[state_off++] = '}';
        state_buf[state_off]   = '\0';
    }
    sprintf(tmp, "{\"ok\":1,%s", state_buf + 1);
    Web_ReplyJson(link, tmp, (uint16_t)strlen(tmp));
}

/* ---------------- 预检 ---------------- */

static void ReplyNoContent(uint8_t link)
{
    char     head[220];
    uint16_t hlen;

    hlen = (uint16_t)sprintf(head,
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET,POST,OPTIONS\r\n"
        "Access-Control-Allow-Headers: *\r\n"
        "Access-Control-Max-Age: 600\r\n"
        "Connection: close\r\n\r\n");

    HttpSend(link, head, hlen);
    Web_CloseLink(link);
}

/* ---------------- 联网 ---------------- */
uint8_t Web_Init(void)
{
    uint8_t i;

    for (i = 0; i < 10; i++) {
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
    while ((uint32_t)(ESP8266_GetTick() - start) < 15000) {
        if (ESP8266_Contains("GOT IP") || ESP8266_Contains("OK"))    return 1;
        if (ESP8266_Contains("FAIL")   || ESP8266_Contains("ERROR")) return 0;
    }
    return 0;
}

uint8_t Web_GetIp(char *ip, uint8_t max_len)
{
    uint8_t i;

    for (i = 0; i < 3; i++) {
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
    ESP8266_SendAT("AT+CIPMUX=1");
    if (!ESP8266_WaitResponse("OK", 2000)) return 0;

    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CIPSERVER=0");
    ESP8266_WaitResponse("OK", 1000);

    sprintf(cmd, "AT+CIPSERVER=1,%u", (unsigned)port);
    ESP8266_ClearBuffer();       /* 启动/自愈期，此时没有要处理的请求，整块清是安全的 */
    ESP8266_SendAT(cmd);
    return ESP8266_WaitResponse("OK", 3000);
}

/**
  * @brief  自愈：关掉所有残链接 + 重开 HTTP 服务器
  * @note   只在"长时间没人访问"时调用。残连接会占满模块的 5 个 link 槽位，
  *         占满后服务器就不再 accept 新连接（表现：连第一次请求都超时），
  *         而 srv_ok 一旦为 1 不会自己复查 —— 靠这个函数把它拉回来
  */
uint8_t Web_ResetServer(uint16_t port)
{
    ESP8266_CloseAllLinks();
    return Web_OpenServer(port);
}

/* ---------------- 路由分发 ---------------- */

static const char json_deny[] = "{\"err\":1,\"need\":\"token\"}";
static const char json_api[]  = "{\"api\":\"stm32f407-esp8266\",\"routes\":["
    "\"/data\",\"/led1/1\",\"/led1/0\",\"/led2/1\",\"/led2/0\","
    "\"/led3/1\",\"/led3/0\",\"/flow/1\",\"/flow/0\","
    "\"/fan/0\",\"/fan/1\",\"/fan/2\",\"/beep\","
    "\"/flash/id\",\"/flash/read\",\"/flash/mcu\",\"/flash/erase\",\"/flash/status\"]}";

static uint8_t HandleOne(void)
{
    static char req[256];
    uint16_t    n;
    uint8_t     link = 0;
    char        path[24];

    n = ESP8266_TakeIp(&link, req, sizeof(req));
    if (n == 0) return 0;

    req_n++;

    if (strstr(req, "OPTIONS") != 0) {
        ReplyNoContent(link);
        return 1;
    }

    if (!Web_TokenOk(req)) {
        Web_ReplyJson(link, json_deny, (uint16_t)(sizeof(json_deny) - 1));
        return 1;
    }

    if (!Web_PathOf(req, path, sizeof(path))) {
        Web_CloseLink(link);
        return 1;
    }

    if      (path[0] == '\0')                    Web_ReplyJson(link, json_api, (uint16_t)(sizeof(json_api) - 1));
    else if (strcmp(path, "data") == 0)          Web_SendAllState(link);
    else if (Led_WebRoute(link, path, req))      ;
    else if (Fan_WebRoute(link, path, req))      ;
    else if (Sensor_WebRoute(link, path, req))   ;
    else if (Beep_WebRoute(link, path, req))     ;
    else if (Flash_WebRoute(link, path, req))    ;
    else if (strcmp(path, "favicon.ico") == 0)   Web_CloseLink(link);
    else                                         Web_ReplyErr(link);
    return 1;
}

void Web_Task(void)
{
    uint8_t n = 0;

    while (n < 4 && HandleOne()) n++;

    if (ESP8266_Find("+IPD,") == 0)
        ESP8266_ClearBuffer();
}

uint16_t Web_ReqCount(void)
{
    return req_n;
}