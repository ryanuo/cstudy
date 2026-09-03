#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdint.h>
#include <stdbool.h>

void Key_Init(void);
uint8_t Key_GetNum(void);

void Key_led_toggle_init(void);

#endif /* __BUTTON_H__ */