#ifndef __FLASH__H_
#define __FLASH__H_
#include "stm32f4xx.h"                  // Device header
void FLASH_write(uint32_t data);
void FLASH_erase(void);
uint32_t FLASH_read(uint32_t address);
#endif
