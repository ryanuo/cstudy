#ifndef __APP_UI_H
#define __APP_UI_H

#include "main.h"
#include "bsp_dht11.h"

/* ============================================================
 *  LCD 界面层：只负责"把状态画出来"
 *  （原来混在 app_wifi.c / app_onenet.c 里的界面都搬到这儿）
 * ============================================================ */

void UI_Boot(void); /* 开机清屏（LCD_Init 由 main 里的既有流程负责） */

/* ---- WiFi 阶段 ---- */
void UI_WifiConnecting(const char *ssid);
void UI_WifiOk(const char *ip);
/* stage: 出错环节描述；code: 状态码；raw: ESP 原始缓冲（可为 NULL） */
void UI_WifiFail(const char *stage, int code, const char *raw);

/* ---- OneNET 阶段 ---- */
void UI_OnenetConnecting(const char *product_id, const char *device_id);
void UI_OnenetOk(const char *device_id);
void UI_OnenetFail(const char *stage, int code, const char *raw);

/* ---- 数据区 ---- */
void UI_ShowTempHumi(const DHT11_Data_t *d);
void UI_ShowTempFail(void);

#endif /* __APP_UI_H */
