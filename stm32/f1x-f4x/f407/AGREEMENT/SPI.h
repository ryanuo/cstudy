#ifndef __SPI_H
#define __SPI_H

#include "stm32f4xx.h"
void SPI1_init(void);
void W25QXX_ReadID(uint8_t *id);
#endif