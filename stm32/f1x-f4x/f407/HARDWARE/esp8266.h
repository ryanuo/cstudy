#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f4xx.h"

/* 串口接收缓冲区大小（环形缓冲，中断里写） */
#define ESP8266_RX_BUF_SIZE  512

/* 函数声明 */
void     ESP8266_Init(void);
void     ESP8266_SendAT(char *cmd);
void     ESP8266_SendData(uint8_t *data, uint16_t len);
uint8_t  ESP8266_WaitResponse(char *expected, uint32_t timeout_ms);
void     ESP8266_ClearBuffer(void);

/* ---- 诊断用 ---- */
/* 改 USART3 的波特率（扫描模块真实波特率） */
void     ESP8266_SetBaud(uint32_t baud);
/* 片内回环自测：把 CR3 的 HDSEL 置 1，TX 与 RX 在芯片内部相连，
   不需要任何外部接线即可验证 "USART3 + 中断 + 缓冲 + 匹配" 整条链路 */
uint8_t  ESP8266_SelfLoopTest(char *expected, uint32_t timeout_ms);
/* 把 PB11 临时配成下拉输入读电平：1 = 线上有东西在主动拉高（模块 TX 空闲应为高） */
uint8_t  ESP8266_ProbeRxPin(void);
/* 看一眼当前收到的原始数据，不消费缓冲 */
uint16_t ESP8266_Peek(uint8_t *dst, uint16_t max_len);
/* 缓冲满被丢掉的字节数（一直为 0 才正常） */
uint16_t ESP8266_LostCount(void);

#endif
