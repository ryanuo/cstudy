#ifndef __USART__H_
#define __USART__H_
#include "stm32f4xx.h"                  // Device header
void USART1_init(void);
void USART1_SendData(uint8_t data);
void USART1_SendString(char*string);
#endif
