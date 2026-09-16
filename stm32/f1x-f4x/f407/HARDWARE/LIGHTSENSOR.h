#ifndef _LIGHTSENSOR_H_
#define _LIGHTSENSOR_H_
#include "stm32f4xx.h"                  // Device header
void LIGHTSENSOR_init(void);
uint8_t LIGHTSENSOR_getvalue(void);
#endif