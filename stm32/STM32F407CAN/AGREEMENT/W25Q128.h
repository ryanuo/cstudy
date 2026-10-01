#ifndef __W25Q128__H_
#define __W25Q128__H_
#include "stm32f4xx.h"                  // Device header
uint16_t W25Q128_getmanddevice(void);
void W25Q128_erasesector(uint32_t address);
void W25Q128_writedata(uint32_t address,uint8_t *buff,uint8_t length);
void W25Q128_readdata(uint32_t address,uint8_t *buff,uint8_t length);
#endif
