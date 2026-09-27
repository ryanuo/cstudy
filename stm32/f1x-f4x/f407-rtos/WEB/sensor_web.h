#ifndef __SENSOR_WEB_H
#define __SENSOR_WEB_H

#include "stm32f4xx.h"

void    Sensor_WebState(void);
uint8_t Sensor_WebRoute(uint8_t link, const char *path, char *req);

#endif