#ifndef __BSP_ESP8266_H
#define __BSP_ESP8266_H

#include "main.h"
#include <stdint.h>

/* ============ 可配置参数 ============ */
#define ESP8266_RX_BUF_SIZE     1024    // 环形缓冲大小
#define ESP8266_ACC_SIZE        2048    // 累积文本缓冲大小

/* ============ 中断接收单字节（给 main.c 用） ============ */
extern uint8_t esp_rx_byte;

/* ============ 对外接口 ============ */

/* 初始化：传入 CubeMX 生成的 UART 句柄（如 &huart3） */
void     BSP_ESP8266_Init(UART_HandleTypeDef *huart);

/* 发送 AT 指令（自动追加 \r\n） */
void     BSP_ESP8266_SendAT(char *cmd);

/* 发送原始数据（用于发 HTTP 响应体等） */
void     BSP_ESP8266_SendData(uint8_t *data, uint16_t len);

/* 中断回调里调用（收到一个字节） */
void     BSP_ESP8266_RxCallback(uint8_t data);

/* 清空所有缓冲（发 AT 指令前调用） */
void     BSP_ESP8266_ClearBuffer(void);

/* 当前缓冲里是否包含某字符串，返回 1/0 */
uint8_t  BSP_ESP8266_Contains(char *expected);

/* 在缓冲里查找字符串，返回指针（找不到返回 NULL） */
char    *BSP_ESP8266_Find(char *pattern);

#endif /* __BSP_ESP8266_H */