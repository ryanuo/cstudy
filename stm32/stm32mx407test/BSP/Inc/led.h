#ifndef __LED_H
#define __LED_H

#include "stm32f4xx_hal.h"

void LED_On(uint8_t number);
void LED_Off(uint8_t number);
void LED_Toggle(uint8_t number);

#endif