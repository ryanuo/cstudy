#ifndef __SPI_H
#define __SPI_H

#include "stm32f4xx.h"
void SPI1_init(void);
void W25QXX_ReadID(uint8_t *id);
void W25QXX_Read(uint8_t *buf, uint32_t addr, uint16_t len);
#endif