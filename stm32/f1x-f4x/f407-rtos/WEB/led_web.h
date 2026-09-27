#ifndef __LED_WEB_H
#define __LED_WEB_H

#include "stm32f4xx.h"

void    Led_WebState(void);
uint8_t Led_WebRoute(uint8_t link, const char *path, char *req);

#endif