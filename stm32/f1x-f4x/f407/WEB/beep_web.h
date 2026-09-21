#ifndef __BEEP_WEB_H
#define __BEEP_WEB_H

#include "stm32f4xx.h"

void    Beep_WebState(void);
uint8_t Beep_WebRoute(uint8_t link, const char *path, char *req);

#endif