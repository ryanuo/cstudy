#ifndef __LIGHT_H
#define __LIGHT_H

#include <stdint.h>

void Light_Init(void);
uint16_t Light_GetValue(void);
void Light_AutoLED(void);

#define THRESHOLD_DARK   2000

#endif
