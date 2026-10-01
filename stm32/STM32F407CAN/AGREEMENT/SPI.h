#ifndef __SPI__H_
#define __SPI__H_
#include "stm32f4xx.h"                  // Device header
void SPI1SOFTWARE_init(void);
uint8_t SPI1_sendbyte(uint8_t byte);
#endif
