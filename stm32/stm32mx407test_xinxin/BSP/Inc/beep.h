#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f4xx_hal.h"

/* 引脚 / 有效电平见 BSP/Inc/board_pins.h（唯一真值源） */

void BEEP_on(void);
void BEEP_off(void);

/* 按有效电平统一入口：on != 0 响 */
void BEEP_Set(uint8_t on);

#endif
