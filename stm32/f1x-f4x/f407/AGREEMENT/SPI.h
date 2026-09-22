#ifndef __SPI_H
#define __SPI_H

#include "stm32f4xx.h"
void SPI1_init(void);
uint8_t SPI1_sendbyte(uint8_t byte);
#endif