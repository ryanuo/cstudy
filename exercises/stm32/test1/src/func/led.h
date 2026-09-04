#ifndef __LED_H__
#define __LED_H__

#include "stm32f1xx_hal.h"

// 定义 LED 逻辑编号
typedef enum {
    LED_USER = 0,    // PA2 外接 LED
    LED_BOARD,       // PC13 板载 LED
    LED_COUNT        // 自动计算 LED 总数
} LED_Id_t;

void LED_Init(void);
void LED_On(LED_Id_t id);
void LED_Off(LED_Id_t id);
void LED_Toggle(LED_Id_t id);

#endif /* __LED_H__ */