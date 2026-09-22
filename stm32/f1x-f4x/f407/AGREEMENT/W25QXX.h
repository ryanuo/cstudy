#ifndef __W25QXX_H
#define __W25QXX_H

#include "stm32f4xx.h"
#include "SPI.h"

void W25QXX_ReadID(uint8_t *id);
void W25QXX_Read(uint8_t *buf, uint32_t addr, uint16_t len);
void W25QXX_Erase_Sector(uint32_t addr);
uint8_t W25QXX_ReadSR(void);

#endif