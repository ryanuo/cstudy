#ifndef __AT24C02__H_
#define __AT24C02__H_
#include "stm32f4xx.h"                  // Device header
int8_t AT24C02_pagewrite(uint8_t slave,uint8_t address,uint8_t *buff,uint8_t length);
int8_t AT24C02_randomread(uint8_t slave,uint8_t address,uint8_t *buff,uint8_t length);
#endif
