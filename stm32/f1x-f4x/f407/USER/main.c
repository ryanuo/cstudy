#include "stm32f4xx.h"
#include "OLED.h"
#include "LED.h"
#include "esp8266.h"

/* ==========================================================================
 * ESP8266 (ESP-01S) 连 WiFi —— 带诊断版 v3
 *
 * 屏幕各行含义：
 *   y16  AT version:xxxx   模块的 AT 固件版本（读 AT+GMR）
 *   y24  SCAN n=08 MODE:OK 扫描到几个热点（AT+CWLAP 的条数）+ CWMODE=1 是否成功
 *   y32  SSID FOUND        目标 SSID 在 2.4G 扫描结果里出现了
 *        SSID NOT FOUND    模块没扫到这个名字 -> 多半是 5G / 名字大小写不对 / 太远 /
 *                          热点当时没开 / 隐藏了 SSID
 *   y40  CWJAP OK / FAIL   连接结果
 *   y48/56 原始回复的 ASCII（不可打印显示为 .）
 *
 *   若 SSID NOT FOUND：屏幕进入循环模式，把模块能看见的热点名字一个个显示出来
 *   （1.5 秒一个，编号 1: 2: 3: ...），照着重填 WIFI_SSID 即可。
 *
 * 板子丝印 LED0(PF9) 亮 = AT 通了；板子 LED1(PF10) 亮 = CWMODE 成功；
 * PE13 亮 = 连上并拿到 IP
 * ========================================================================== */

#define WIFI_SSID     "YQ-shixun7"    /* 必须 2.4G，大小写要和热点完全一致 */
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
    uint8_t  buf[42];
    uint16_t n = ESP8266_Peek(buf, 40);

    if (n == 0)
    {
        OLED_ShowString(0, 48, "n=000 (no data)     ", OLED_6X8);
        OLED_ShowString(0, 56, "                    ", OLED_6X8);
        return;
    }
    OLED_ShowAscii(48, buf, (n > 20) ? 20 : n);
    OLED_ShowAscii(56, buf + 20, (n > 20) ? ((n > 40) ? 20 : n - 20) : 0);
}

/* 把 src 的前 20 个字符拷进 dst 并补 '\0'
   （OLED_ShowString 会一直画到 '\0'，直接传累积缓冲里的指针会把几百个字符全画到屏上） */
static void Copy20(char *dst, char *src)
{
    uint8_t k = 0;

    if (src == 0) { dst[0] = '\0'; return; }
    while (k < 20 && src[k] != '\0') { dst[k] = src[k]; k++; }
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

int main(void)
{
    uint8_t  at_ok = 0, mode_ok = 0, ssid_ok = 0;
    uint16_t ap_n = 0;
    uint8_t  i, j, tick = 0;
    char     verbuf[24];
    char     name[24];

    LED_init();
    ESP8266_Init();                 /* USART3 + 1ms 滴答都起好 */
    OLED_Init();
    OLED_Clear();

    OLED_ShowString(0, 0, "WIFI DIAG", OLED_8X16);
    OLED_ShowString(0, 16, "wait boot 1.5s      ", OLED_6X8);
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
        OLED_ShowLastRx();
        OLED_Update();
        while (1) { LED2_on(); ESP8266_DelayMs(300); LED2_off(); ESP8266_DelayMs(300); }
    }

    LED1_on();                                            /* 板子 LED0 (PF9) 亮 */
    OLED_ShowString(0, 16, "AT OK               ", OLED_6X8);
    OLED_Update();

    /* ---------- 2. 关回显（回复更干净） ---------- */
    ESP8266_ClearBuffer();
    ESP8266_SendAT("ATE0");
    ESP8266_WaitResponse("OK", 2000);

    /* ---------- 3. 读 AT 固件版本 ---------- */
    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+GMR");
    ESP8266_WaitResponse("OK", 3000);
    Copy20(verbuf, ESP8266_Find("AT version:"));          /* 从累积文本里截出版本串 */
    if (verbuf[0] == '\0') Copy20(verbuf, "AT version: n/a");
    OLED_ShowString(0, 16, verbuf, OLED_6X8);
    OLED_Update();

    /* ---------- 4. Station 模式 ---------- */
    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CWMODE=1");
    mode_ok = ESP8266_WaitResponse("OK", 2000);
    if (mode_ok) LED2_on();                               /* 板子 LED1 (PF10) 亮 */

    /* ---------- 5. 扫描附近热点，看目标 SSID 在不在 2.4G 里 ---------- */
    ESP8266_ClearBuffer();
    ESP8266_SendAT("AT+CWLAP");
    ESP8266_WaitResponse("OK", 20000);                    /* 扫描一般 2~8 秒 */
    ap_n    = ESP8266_Count("+CWLAP:");
    ssid_ok = ESP8266_Contains(WIFI_SSID);

    OLED_ShowString(0, 24, "SCAN n=   MODE:     ", OLED_6X8);
    OLED_ShowNum(48, 24, ap_n, 2, OLED_6X8);
    OLED_ShowString(78, 24, mode_ok ? "OK" : "--", OLED_6X8);
    OLED_ShowString(0, 32, ssid_ok ? "SSID FOUND          " : "SSID NOT FOUND      ", OLED_6X8);
    OLED_Update();

    /* ---------- 6. 没扫到目标名字：把模块能看见的热点名字循环显示 ---------- */
    if (ssid_ok == 0)
    {
        OLED_ShowString(0, 40, "AP list ->          ", OLED_6X8);
        OLED_ShowString(0, 56, "edit WIFI_SSID      ", OLED_6X8);
        OLED_Update();

        while (1)
        {
            for (j = 0; j < ap_n && j < 20; j++)
            {
                char nm[16];
                uint8_t k;

                OLED_ShowString(0, 48, "                    ", OLED_6X8);   /* 清行 */
                OLED_ShowNum(0, 48, j + 1, 2, OLED_6X8);                /* 编号 01 02 ... */
                OLED_ShowString(12, 48, ":", OLED_6X8);

                nm[0] = '\0';
                if (ESP8266_GetSsid(j, name, sizeof(name)))
                {
                    for (k = 0; k < 15 && name[k] != '\0'; k++) nm[k] = name[k];
                    nm[k] = '\0';
                }
                OLED_ShowString(18, 48, nm, OLED_6X8);
                OLED_Update();
                ESP8266_DelayMs(1500);
            }
            ESP8266_DelayMs(500);
        }
    }

    /* ---------- 7. SSID 扫到了：正式连接 ---------- */
    wifi_ok = WIFI_Connect();
    if (wifi_ok)
    {
        LED3_on();                                        /* PE13 亮 = 连上并拿到 IP */
        OLED_ShowString(0, 40, "CWJAP OK            ", OLED_6X8);
        ESP8266_ClearBuffer();
        ESP8266_SendAT("AT+CIFSR");
        ESP8266_WaitResponse("OK", 3000);
        OLED_ShowLastRx();                                /* 显示 IP */
    }
    else
    {
        OLED_ShowString(0, 40, "CWJAP FAIL          ", OLED_6X8);
        OLED_ShowLastRx();                                /* 显示模块的原话 */
    }
    OLED_Update();

    /* ---------- 8. 失败就每 10 秒自动重试 ---------- */
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
                    OLED_ShowString(0, 40, "CWJAP OK            ", OLED_6X8);
                    OLED_ShowLastRx();
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
