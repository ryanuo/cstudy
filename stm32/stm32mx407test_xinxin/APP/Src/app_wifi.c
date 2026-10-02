#include "app_wifi.h"
#include "bsp_esp8266.h"
#include "oled.h"

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

/* ================= 对外接口 ================= */

WIFI_Status_t WIFI_Init(void) {
  BSP_ESP8266_ClearBuffer();
  BSP_ESP8266_SendAT("AT");
  if (!wifi_wait_for("OK", "ERROR", NULL, 1000))
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
    HAL_Delay(500);
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

WIFI_Status_t WIFI_TestAndShow(char *ssid, char *pwd) {
  char ip[32];
  char line[64];
  WIFI_Status_t st;

  /* 1. 初始化 */
  OLED_Clear();
  OLED_ShowString(0, 0, "Init ESP8266...", OLED_8X16);
  st = WIFI_Init();
  if (st != WIFI_OK) {
    OLED_Clear();
    OLED_ShowString(0, 0, "No module!", OLED_8X16);
    return st;
  }

  /* 2. 连 WiFi */
  OLED_Clear();
  OLED_ShowString(0, 0, "Connecting...", OLED_8X16);
  snprintf(line, sizeof(line), "SSID:%s", ssid);
  OLED_ShowString(0, 16, line, OLED_8X16);

  st = WIFI_Connect(ssid, pwd);
  if (st != WIFI_OK) {
    OLED_Clear();
    OLED_ShowString(0, 0, "Connect fail", OLED_8X16);
    return st;
  }

  /* 3. 拿 IP */
  OLED_Clear();
  OLED_ShowString(0, 0, "Getting IP...", OLED_8X16);
  st = WIFI_GetIP(ip, sizeof(ip));
  if (st != WIFI_OK) {
    OLED_Clear();
    OLED_ShowString(0, 0, "Get IP fail", OLED_8X16);
    return st;
  }

  /* 4. 显示结果 */
  OLED_Clear();
  OLED_ShowString(0, 0, "WiFi OK", OLED_8X16);
  snprintf(line, sizeof(line), "IP:%s", ip);
  OLED_ShowString(0, 16, line, OLED_8X16);

  return WIFI_OK;
}