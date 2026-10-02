#ifndef __APP_WIFI_H
#define __APP_WIFI_H

#include "main.h"

/* 状态枚举：一看就懂 */
typedef enum {
  WIFI_OK = 0,           // 成功
  WIFI_ERR_NO_MODULE,    // ESP8266 没响应
  WIFI_ERR_AT_FAIL,      // AT 指令返回 ERROR
  WIFI_ERR_CONNECT_FAIL, // 连 WiFi 失败（密码错/信号弱）
  WIFI_ERR_NO_IP,        // 连上了但拿不到 IP
  WIFI_ERR_TIMEOUT       // 超时
} WIFI_Status_t;

/* ---------- 对外接口 ---------- */

/**
 * @brief  初始化 ESP8266，并检测模块是否在线
 * @retval WIFI_OK / WIFI_ERR_NO_MODULE
 */
WIFI_Status_t WIFI_Init(void);

/**
 * @brief  连接指定 WiFi
 * @param  ssid   WiFi 名（不带引号）
 * @param  pwd    WiFi 密码（不带引号）
 * @retval WIFI_OK / WIFI_ERR_CONNECT_FAIL / WIFI_ERR_TIMEOUT
 */
WIFI_Status_t WIFI_Connect(char *ssid, char *pwd);

/**
 * @brief  获取当前 IP 地址
 * @param  ip_buf  输出缓冲
 * @param  len     缓冲长度（建议 >= 32）
 * @retval WIFI_OK / WIFI_ERR_NO_IP
 */
WIFI_Status_t WIFI_GetIP(char *ip_buf, uint8_t len);

/**
 * @brief  综合测试：连 WiFi + 拿 IP + OLED 显示
 * @param  ssid, pwd   WiFi 凭据
 * @retval WIFI_OK 或错误码
 * @note   需要 OLED 驱动支持
 */
WIFI_Status_t WIFI_TestAndShow(char *ssid, char *pwd);

#endif