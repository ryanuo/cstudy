#ifndef _LIGHTSENSOR_H_
#define _LIGHTSENSOR_H_
#include "stm32f4xx.h" // Device header
void LIGHT_Init(void);
uint16_t LIGHT_GetValue(void);
#endif