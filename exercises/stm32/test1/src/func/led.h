#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"

typedef enum {
    LED1 = 0,    // PA0
    LED2,        // PA1
    LED3,        // PA2
    LED_COUNT
} LED_Id_t;

void LED_Init(void);
void LED_On(LED_Id_t id);
void LED_Off(LED_Id_t id);
void LED_Toggle(LED_Id_t id);
void LED_SetMode(uint8_t mode);
void LED_Update(void);

#endif
