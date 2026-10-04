#include "app_ui.h"
#include "lcd.h"

#include <stdio.h>
#include <string.h>

/* ============================================================
 *  屏幕布局（128x160 竖屏，坐标沿用原工程的观感）
 *    20  : 标题（16 号字）
 *    50  : 副标题 / 关键信息
 *    70/100 : 追加信息
 *    60..160: 失败诊断时摊开的原始字节
 *    130 : 温湿度数据行
 * ============================================================ */
#define UI_TITLE_Y 20
#define UI_LINE1_Y 50
#define UI_LINE2_Y 70
#define UI_LINE3_Y 100
#define UI_DATA_Y 130
#define UI_DATA_H 16

/* ============================================================
 *  原始字节摊开显示（诊断用）
 *  全是乱码   -> 线上不是 UART 信号（P5 短路帽没拆 / PHY 占用 PA1）或波特率不对
 *  有 AT/OK   -> 解析问题
 *  只有 AT    -> TX 被回环 / 回显没关
 * ============================================================ */
static void ui_dump_bytes(const char *buf, uint16_t n, uint16_t from, uint16_t len,
                          uint16_t y, uint8_t hex) {
  char line[44];
  uint16_t i;
  uint16_t k = 0;

  line[0] = '\0';
  if (n > 0 && from < n) {
    for (i = from; i < n && i < (uint16_t)(from + len); i++) {
      if (hex) {
        if (k + 4 >= sizeof line)
          break;
        k += (uint16_t)snprintf(line + k, sizeof line - k, "%02X ",
                                (unsigned char)buf[i]);
      } else {
        char c = buf[i];
        if (k + 2 >= sizeof line)
          break;
        line[k++] = (c >= 0x20 && c <= 0x7e) ? c : '.';
      }
    }
    line[k] = '\0';
  }
  LCD_DisplayString_color(4, y, 12, (u8 *)line, BLACK, WHITE);
}

/* 头尾各来一段：头部是模块启动日志，尾部是最后收到的内容 */
static void ui_dump_raw(const char *buf) {
  char line[24];
  uint16_t n;

  if (buf == NULL)
    return;

  n = (uint16_t)strlen(buf);
  snprintf(line, sizeof line, "RX=%u", (unsigned)n);
  LCD_DisplayString_color(4, 60, 16, (u8 *)line, RED, WHITE);

  ui_dump_bytes(buf, n, 0, 13, 84, 1);
  ui_dump_bytes(buf, n, 0, 39, 98, 0);
  ui_dump_bytes(buf, n, 39, 39, 112, 0);

  ui_dump_bytes(buf, n, (n > 26) ? (uint16_t)(n - 26) : 0, 13, 128, 1);
  ui_dump_bytes(buf, n, (n > 78) ? (uint16_t)(n - 78) : 0, 39, 142, 0);
  ui_dump_bytes(buf, n, (n > 39) ? (uint16_t)(n - 39) : 0, 39, 156, 0);
}

/* ============================================================
 *  对外接口
 * ============================================================ */
void UI_Boot(void) {
  LCD_Clear(WHITE);
  LCD_Showf(20, UI_TITLE_Y, 16, BLACK, WHITE, "Init ESP8266...");
}

void UI_WifiConnecting(const char *ssid) {
  LCD_Clear(WHITE);
  LCD_Showf(20, UI_TITLE_Y, 16, BLACK, WHITE, "Connecting...");
  LCD_Showf(20, UI_LINE1_Y, 16, BLACK, WHITE, "SSID:%s",
            ssid ? ssid : "(null)");
}

void UI_WifiOk(const char *ip) {
  LCD_Clear(WHITE);
  LCD_Showf(20, UI_TITLE_Y, 24, GREEN, WHITE, "WiFi OK");
  LCD_Showf(20, UI_LINE2_Y, 16, BLACK, WHITE, "IP:%s", ip ? ip : "-");
}

void UI_WifiFail(const char *stage, int code, const char *raw) {
  LCD_Clear(WHITE);
  LCD_Showf(20, UI_TITLE_Y, 16, RED, WHITE, "%s fail",
            stage ? stage : "WiFi");
  LCD_Showf(20, UI_LINE1_Y, 16, RED, WHITE, "Err:%d", code);
  ui_dump_raw(raw);
}

void UI_OnenetConnecting(const char *product_id, const char *device_id) {
  LCD_Clear(WHITE);
  LCD_Showf(20, UI_TITLE_Y, 16, BLACK, WHITE, "OneNET...");
  LCD_Showf(20, UI_LINE1_Y, 16, BLACK, WHITE, "PID:%s",
            product_id ? product_id : "-");
  LCD_Showf(20, UI_LINE2_Y, 16, BLACK, WHITE, "DID:%s",
            device_id ? device_id : "-");
}

void UI_OnenetOk(const char *device_id) {
  LCD_Clear(WHITE);
  LCD_Showf(20, UI_TITLE_Y, 24, GREEN, WHITE, "OneNET OK");
  LCD_Showf(20, UI_LINE2_Y, 16, BLACK, WHITE, "Device Online");
  LCD_Showf(20, UI_LINE3_Y, 16, BLACK, WHITE, "DID:%s",
            device_id ? device_id : "-");
}

void UI_OnenetFail(const char *stage, int code, const char *raw) {
  LCD_Clear(WHITE);
  LCD_Showf(20, UI_TITLE_Y, 16, RED, WHITE, "%s fail", stage ? stage : "MQTT");
  LCD_Showf(20, UI_LINE1_Y, 16, RED, WHITE, "Err:%d", code);
  ui_dump_raw(raw);
}

void UI_ShowTempHumi(const DHT11_Data_t *d) {
  if (d == NULL)
    return;

  LCD_Fill_onecolor(20, UI_DATA_Y, 230, UI_DATA_Y + UI_DATA_H, WHITE);
  LCD_Showf(20, UI_DATA_Y, 16, BLACK, WHITE, "T:%d.%d C  H:%d.%d %%", d->temp_int,
            d->temp_dec, d->humi_int, d->humi_dec);
}

void UI_ShowTempFail(void) {
  LCD_Fill_onecolor(20, UI_DATA_Y, 230, UI_DATA_Y + UI_DATA_H, WHITE);
  LCD_Showf(20, UI_DATA_Y, 16, RED, WHITE, "DHT11 read fail");
}
