#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

void UART3_Init(void);
void UART3_SendChar(char c);
void UART3_SendString(const char *str);
void UART3_SendCharRaw(char c);
void UART3_SendStringRaw(const char *str);
HAL_StatusTypeDef UART3_SendCharWithStatus(char c);

#endif
