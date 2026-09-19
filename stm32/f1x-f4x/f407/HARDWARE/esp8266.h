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

/* 诊断用：把 USART3 的波特率改成 baud（扫描模块真实波特率用） */
void     ESP8266_SetBaud(uint32_t baud);

/* 诊断用：看一眼当前收到的原始数据，不消费缓冲 */
uint16_t ESP8266_Peek(uint8_t *dst, uint16_t max_len);
/* 诊断用：缓冲满被丢掉的字节数（一直为 0 才正常） */
uint16_t ESP8266_LostCount(void);

#endif
