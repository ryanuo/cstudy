/**
  ******************************************************************************
  * @file    led.h
  * @brief   LED 驱动模块头文件
  ******************************************************************************
  * 硬件连接:
  *   LED0 -> PF9  (低电平点亮)
  *   LED1 -> PF10 (低电平点亮)
  *   LED2 -> PE13 (低电平点亮)
  *   LED3 -> PE14 (低电平点亮)
  */

#ifndef __LED_H
#define __LED_H

#include "stdint.h"

/* LED 数量 */
#define LED_NUM     4

/* LED 索引 */
#define LED0        0
#define LED1        1
#define LED2        2
#define LED3        3

/* LED 状态 */
#define LED_OFF     0
#define LED_ON      1

void LED_Init(void);
void LED_On(uint8_t idx);
void LED_Off(uint8_t idx);
void LED_Toggle(uint8_t idx);
void LED_SetAll(uint8_t state);
void LED_Waterfall(uint16_t interval_ms);

#endif /* __LED_H */
