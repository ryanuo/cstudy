#ifndef __WEB_H
#define __WEB_H

#include "stm32f4xx.h"

/*
 * 手机网页控制 —— HTTP/TCP 服务器层
 * 依赖：esp8266.c（USART3 上的 AT 指令）、LED/BEEP/ADC/LIGHTSENSOR
 *
 * 路由：
 *   /            网页（Vue 3 走 CDN，板子只发这一小段 HTML）
 *   /data        JSON：{"led0":0,"led1":1,"light":1234,"pot":2048,"req":12}
 *   /led0/1|0    板子丝印 LED0（PF9）开/关
 *   /led1/1|0    板子丝印 LED1（PF10）开/关
 *   /beep        蜂鸣器响 200ms
 *   其他         404 JSON
 */

/* 串口通了（AT 重试 + CWMODE=1）返回 1 */
uint8_t  Web_Init(void);
/* 连热点，成功返回 1 */
uint8_t  Web_Connect(char *ssid, char *pass);
/* 读 IP（兼容 AT+CIFSR 新旧格式），成功返回 1 */
uint8_t  Web_GetIp(char *ip, uint8_t max_len);
/* 开 TCP 服务器（AT+CIPMUX=1 + AT+CIPSERVER=1,port），成功返回 1 */
uint8_t  Web_OpenServer(uint16_t port);
/* 主循环里周期调用：处理一次浏览器请求 */
void     Web_Task(void);
/* 已处理的请求数（给 OLED 显示） */
uint16_t Web_ReqCount(void);

#endif
