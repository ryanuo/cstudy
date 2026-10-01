#ifndef __ONEWIREBUS__H_
#define __ONEWIREBUS__H_
#include "stm32f4xx.h"                  // Device header
int8_t DHT11_gettemphum(uint8_t *buff);
void DHT11_init(void);
#endif
