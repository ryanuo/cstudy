#ifndef __LED_H
#define __LED_H

#include "stm32f4xx_hal.h"

/* 引脚 / 有效电平见 BSP/Inc/board_pins.h（唯一真值源） */

/* number = 0 .. BOARD_LED_COUNT-1，越界直接忽略 */
void LED_On(uint8_t number);
void LED_Off(uint8_t number);
void LED_Toggle(uint8_t number);

/* 按有效电平统一入口：on != 0 点亮 */
void LED_Set(uint8_t number, uint8_t on);

/* 读回引脚真实电平（1 = 亮）：上电上报真实状态时用 */
uint8_t LED_IsOn(uint8_t number);

#endif
