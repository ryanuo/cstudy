#ifndef __FLASH_WEB_H
#define __FLASH_WEB_H

#include "stm32f4xx.h"

uint8_t Flash_WebRoute(uint8_t link, const char *path, char *req);

#endif