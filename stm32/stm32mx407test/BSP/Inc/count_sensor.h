#ifndef __BSP_COUNT_SENSOR_H
#define __BSP_COUNT_SENSOR_H

#include "stm32f4xx_hal.h"

/* 初始化（可选：清计数、开中断等） */
void CountSensor_Init(void);

/* 获取当前计数值 */
uint32_t CountSensor_GetValue(void);

/* 清零 */
void CountSensor_Reset(void);

/* 给中断回调用的入口（由 HAL_GPIO_EXTI_Callback 调用） */
void CountSensor_EXTI_Callback(uint16_t GPIO_Pin);

#endif