#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f4xx.h"

/* 串口接收缓冲区大小（环形缓冲，中断里写） */
#define ESP8266_RX_BUF_SIZE  512

/* 初始化：USART3(PB10/PB11, 115200-8-N-1) + RXNE 中断 + 1ms 滴答 */
void     ESP8266_Init(void);
/* 发 AT 指令（自动补 \r\n） */
void     ESP8266_SendAT(char *cmd);
/* 发原始数据（透传模式用） */
void     ESP8266_SendData(uint8_t *data, uint16_t len);
/* 清空接收缓冲（发下一条命令前调用） */
void     ESP8266_ClearBuffer(void);
/* 等待响应，真实毫秒超时 */
uint8_t  ESP8266_WaitResponse(char *expected, uint32_t timeout_ms);
/* 已收到的数据里是否包含 expected（不消费） */
uint8_t  ESP8266_Contains(char *expected);
/* 在已收到的数据里找子串，返回指针（找不到返回 0） */
char    *ESP8266_Find(char *pattern);
/* 取一份已收到的数据（不消费），返回字节数 */
uint16_t ESP8266_Peek(uint8_t *dst, uint16_t max_len);

/* 时间基准（SysTick 1ms）。
   注意：启动滴答之后不要再调 SYSTEM/DELAY.c 的 DELAY_ms()——
   它直接开关 SysTick 且 TICKINT=0，会把滴答关掉，延时请用 ESP8266_DelayMs() */
uint32_t ESP8266_GetTick(void);
void     ESP8266_DelayMs(uint32_t ms);

#endif
