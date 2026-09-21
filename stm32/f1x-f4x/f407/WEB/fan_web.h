#ifndef __FAN_WEB_H
#define __FAN_WEB_H

#include "stm32f4xx.h"

void    Fan_WebState(void);
uint8_t Fan_WebRoute(uint8_t link, const char *path, char *req);

#endif