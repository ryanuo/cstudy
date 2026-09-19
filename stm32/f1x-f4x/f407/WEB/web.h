#ifndef __WEB_H
#define __WEB_H

#include "stm32f4xx.h"

/*
 * 纯 JSON 接口层（不带网页）—— 页面放电脑/云上，跨域调用板子
 * 依赖：esp8266.c（USART3 上的 AT 指令）、LED/BEEP/ADC/LIGHTSENSOR
 *
 * 接口（GET，返回 application/json；所有响应都带 Access-Control-Allow-Origin: *）：
 *   /                  接口清单
 *   /data              {"led0":0,"led1":1,"led3":1,"led4":0,"fan":0,
 *                       "light":1234,"pot":2048,"req":12}
 *                      （led0/led1/led3/led4/fan 都是直接读引脚回推的真实状态）
 *   /led0/1 /led0/0    板子丝印 LED0（PF9）开/关 -> {"ok":1}
 *   /led1/1 /led1/0    板子丝印 LED1（PF10）开/关 -> {"ok":1}
 *   /led4/1 /led4/0    板子丝印 FSMC_D11（PE14）开/关 -> {"ok":1}
 *   /fan/0 /fan/1 /fan/2  风扇 L9110H（PC6/PC7）：停 / 正转 / 反转 -> {"ok":1}
 *   /beep              蜂鸣器响 200ms          -> {"ok":1}
 *   OPTIONS 任意路径   204 + CORS 头（跨域预检）
 */

/* 串口通了（AT 重试 + CWMODE=1）返回 1 */
uint8_t  Web_Init(void);
/* 连热点，成功返回 1 */
uint8_t  Web_Connect(char *ssid, char *pass);
/* 读 IP（兼容 AT+CIFSR 新旧格式），成功返回 1 */
uint8_t  Web_GetIp(char *ip, uint8_t max_len);
/* 开 TCP 服务器（AT+CIPMUX=1 + AT+CIPSERVER=1,port），成功返回 1 */
uint8_t  Web_OpenServer(uint16_t port);
/* 主循环里周期调用：处理一次请求 */
void     Web_Task(void);
/* 已处理的请求数（给 OLED 显示） */
uint16_t Web_ReqCount(void);

#endif
