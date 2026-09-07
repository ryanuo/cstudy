#ifndef __KEY_H
#define __KEY_H

#include <stdint.h>

void Key_Init(void);
uint8_t Key_GetNum(void);      // 1=PA3, 2=PA4, 3=PA5
uint8_t Key_PA5_Pressed(void);

void Key_led_toggle_init(void);

#endif
