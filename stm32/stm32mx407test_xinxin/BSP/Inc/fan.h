#ifndef _FAN_H_
#define _FAN_H_
#include "stm32f4xx_hal.h"

/* 引脚 / 有效电平见 BSP/Inc/board_pins.h（唯一真值源）。
 * 引脚的时钟与模式由 CubeMX（MX_GPIO_Init）配置，本驱动只负责写电平。*/

void FAN_forwardrotation(void);
void FAN_reverserotation(void);
void FAN_off(void);

/* 物模型要的开关语义：on != 0 正转，否则停 */
void FAN_Set(uint8_t on);

/* 读回方向线电平（1 = 正在转） */
uint8_t FAN_IsOn(void);

#endif
