#include "app_wifi.h"
#include "bsp_esp8266.h"
#include "lcd.h"

#include <stdio.h>
#include <string.h>

/* ================= 内部私有函数 ================= */

/* 等某个字符串出现，同时监视错误关键字，返回 1=匹配 0=失败/超时 */
static uint8_t wifi_wait_for(char *expect, char *err1, char *err2,
                             uint32_t timeout_ms) {
  uint32_t start = HAL_GetTick();
  while ((HAL_GetTick() - start) < timeout_ms) {
    if (BSP_ESP8266_Contains(expect))
      return 1;
    if (err1 && BSP_ESP8266_Contains(err1))
      return 0;
    if (err2 && BSP_ESP8266_Contains(err2))
      return 0;
  }
  return 0;
}

/* 发一条 AT 指令并等 OK */
static uint8_t wifi_send_at_wait_ok(char *cmd, uint32_t timeout_ms) {
  BSP_ESP8266_ClearBuffer();
  BSP_ESP8266_SendAT(cmd);
  return wifi_wait_for("OK", "ERROR", "FAIL", timeout_ms);
}

/* 从缓冲里抓一个 IPv4 地址，兼容新旧固件
 *   - 新版：+CIFSR:STAIP,"192.168.1.100"
 *   - 旧版：192.168.1.100
 *   - AP+STA：APIP 和 STAIP 两个，取最后一个（一般是 STAIP）
 */
static uint8_t wifi_parse_ip(char *dst, uint8_t max_len) {
  char *p = BSP_ESP8266_Find(""); /* 这里只是触发一次 esp_pump，返回整段 */
  char *found = 0;
  uint8_t found_len = 0, k;

  if (!p) {
    dst[0] = '\0';
    return 0;
  }

  while (*p != '\0') {
    char *q = p;
    uint8_t seg, digits, ok = 1;

    if (!(*p >= '0' && *p <= '9')) {
      p++;
      continue;
    }

    for (seg = 0; seg < 4 && ok; seg++) {
      digits = 0;
      while (*q >= '0' && *q <= '9' && digits < 3) {
        q++;
        digits++;
      }
      if (digits == 0) {
        ok = 0;
        break;
      }
      if (seg < 3) {
        if (*q != '.') {
          ok = 0;
          break;
        }
        q++;
      }
    }
    if (ok && !(*q >= '0' && *q <= '9')) {
      found = p;
      found_len = (uint8_t)(q - p);
      p = q;
    } else {
      p++;
    }
  }

  if (found == 0) {
    dst[0] = '\0';
    return 0;
  }

  k = 0;
  while (k < found_len && k < (uint8_t)(max_len - 1)) {
    dst[k] = found[k];
    k++;
  }
  dst[k] = '\0';
  return 1;
}

/* 把缓冲区里某一段按 hex 或 ASCII 打到屏上（12 号字，一行约 39 字符）
 * hex=1 时 len 用 13（13*3=39 字符），hex=0 时 len 用 39 */
static void wifi_dump(char *buf, uint16_t n, uint16_t from, uint16_t len,
                      uint16_t y, uint8_t hex) {
  char line[44];
  uint16_t i;
  uint8_t k = 0;

  line[0] = '\0';
  if (n > 0 && from < n) {
    for (i = from; i < n && i < (uint16_t)(from + len); i++) {
      if (hex)
        k += (uint8_t)snprintf(line + k, sizeof(line) - k, "%02X ",
                               (unsigned char)buf[i]);
      else {
        char c = buf[i];
        line[k++] = (c >= 0x20 && c <= 0x7e) ? c : '.';
      }
    }
    line[k] = '\0';
  }
  LCD_DisplayString(4, y, 12, (u8 *)line);
}

/* 收到的东西看不懂时，把原始字节摊开看：头尾的 hex + ASCII */
static void wifi_show_raw(char *buf) {
  char line[32];
  uint16_t n = (uint16_t)strlen(buf);

  snprintf(line, sizeof(line), "RX=%u", (unsigned)n);
  LCD_DisplayString(4, 60, 16, (u8 *)line);

  /* 头部（模块上电启动日志一般在这） */
  wifi_dump(buf, n, 0, 13, 84, 1);
  wifi_dump(buf, n, 0, 39, 98, 0);
  wifi_dump(buf, n, 39, 39, 112, 0);

  /* 尾部（最后收到的东西） */
  wifi_dump(buf, n, (n > 26) ? (uint16_t)(n - 26) : 0, 13, 128, 1);
  wifi_dump(buf, n, (n > 78) ? (uint16_t)(n - 78) : 0, 39, 142, 0);
  wifi_dump(buf, n, (n > 39) ? (uint16_t)(n - 39) : 0, 39, 156, 0);
}

/* ================= 对外接口 ================= */

WIFI_Status_t WIFI_Init(void) {
  BSP_ESP8266_ClearBuffer();
  BSP_ESP8266_SendAT("AT");
  /* 模块刚上电时还在启动，给 2 秒；这段时间也能收到它的启动信息 */
  if (!wifi_wait_for("OK", "ERROR", NULL, 2000))
    return WIFI_ERR_NO_MODULE;

  /* 关闭回显，避免影响后续解析 */
  wifi_send_at_wait_ok("ATE0", 500);

  /* 设置 Station 模式 */
  if (!wifi_send_at_wait_ok("AT+CWMODE=1", 1000))
    return WIFI_ERR_AT_FAIL;

  return WIFI_OK;
}

WIFI_Status_t WIFI_Connect(char *ssid, char *pwd) {
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);

  BSP_ESP8266_ClearBuffer();
  BSP_ESP8266_SendAT(cmd);

  /* 连 WiFi 慢，给 15 秒；成功的关键字是 WIFI GOT IP */
  if (wifi_wait_for("WIFI GOT IP", "FAIL", "ERROR", 15000)) {
    HAL_Delay(2000);
    return WIFI_OK;
  }

  return WIFI_ERR_CONNECT_FAIL;
}

WIFI_Status_t WIFI_GetIP(char *ip_buf, uint8_t len) {
  ip_buf[0] = '\0';

  BSP_ESP8266_ClearBuffer();
  BSP_ESP8266_SendAT("AT+CIFSR");

  if (!wifi_wait_for("OK", "ERROR", NULL, 2000))
    return WIFI_ERR_TIMEOUT;

  if (wifi_parse_ip(ip_buf, len))
    return WIFI_OK;
  return WIFI_ERR_NO_IP;
}

/* ================= 综合测试 + OLED 显示 ================= */

#define LCD_CLEAR_SCREEN WHITE
WIFI_Status_t WIFI_TestAndShow(char *ssid, char *pwd) {
  char ip[32];
  char line[64];
  WIFI_Status_t st;

  /* 1. 初始化 */
  LCD_Clear(LCD_CLEAR_SCREEN);
  Text_Foreground_Color(BLACK);
  Text_Background_Color(WHITE);
  LCD_DisplayString(20, 20, 16, (u8 *)"Init ESP8266...");

  st = WIFI_Init();
  if (st != WIFI_OK) {
    LCD_Clear(LCD_CLEAR_SCREEN);
    Text_Foreground_Color(RED);
    Text_Background_Color(WHITE);
    LCD_DisplayString(20, 20, 16, (u8 *)"No module!");
    /* 诊断：把这一轮收到的原始字节摊开
       全是乱码   -> 线上不是 UART 信号（P5 短路帽没拆/PHY 占用 PA1）或波特率不对
       有 AT/OK   -> 解析问题
       只有 AT    -> TX 被回环/回显 */
    wifi_show_raw(BSP_ESP8266_Find(""));
    return st;
  }

  /* 2. 连 WiFi */
  LCD_Clear(LCD_CLEAR_SCREEN);
  Text_Foreground_Color(BLACK);
  Text_Background_Color(WHITE);
  LCD_DisplayString(20, 20, 16, (u8 *)"Connecting...");
  snprintf(line, sizeof(line), "SSID:%s", ssid);
  LCD_DisplayString(20, 60, 16, (u8 *)line);

  st = WIFI_Connect(ssid, pwd);
  if (st != WIFI_OK) {
    LCD_Clear(LCD_CLEAR_SCREEN);
    Text_Foreground_Color(RED);
    Text_Background_Color(WHITE);
    LCD_DisplayString(20, 20, 16, (u8 *)"Connect fail");
    return st;
  }

  /* 3. 拿 IP */
  LCD_Clear(LCD_CLEAR_SCREEN);
  Text_Foreground_Color(BLACK);
  Text_Background_Color(WHITE);
  LCD_DisplayString(20, 20, 16, (u8 *)"Getting IP...");

  st = WIFI_GetIP(ip, sizeof(ip));
  if (st != WIFI_OK) {
    LCD_Clear(LCD_CLEAR_SCREEN);
    Text_Foreground_Color(RED);
    Text_Background_Color(WHITE);
    LCD_DisplayString(20, 20, 16, (u8 *)"Get IP fail");
    return st;
  }

  /* 4. 显示结果 */
  LCD_Clear(LCD_CLEAR_SCREEN);
  Text_Foreground_Color(GREEN);
  Text_Background_Color(WHITE);
  LCD_DisplayString(20, 20, 24, (u8 *)"WiFi OK");

  Text_Foreground_Color(BLACK);
  snprintf(line, sizeof(line), "IP:%s", ip);
  LCD_DisplayString(20, 70, 16, (u8 *)line);

  return WIFI_OK;
}