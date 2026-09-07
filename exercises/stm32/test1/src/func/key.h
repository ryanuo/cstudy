#ifndef __KEY_H
#define __KEY_H

#include <stdint.h>

void Key_Init(void);
uint8_t Key_GetNum(void);      // 1=PA3, 2=PA4, 3=PA5
uint8_t Key_PA5_Pressed(void); // 保留兼容
uint8_t Key_Mode_Pressed(void); // PB0 模式切换

void Key_led_toggle_init(void);

#endif
