#ifndef __UART_CMD_H
#define __UART_CMD_H

#include <stdint.h>

void UART_CMD_Init(void);
void UART_CMD_Process(void);  /* 在 SysTick 中断调用，解析命令 */
uint8_t UART_CMD_GetTargetMode(void);  /* 获取 Qt 设置的目标模式 */
void UART_CMD_SetMode(uint8_t m);      /* 同步当前模式到 uart_cmd */
void UART_CMD_ClearTargetMode(void);   /* 消费后清除目标模式 */

#endif
