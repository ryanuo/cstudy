/**
 ******************************************************************************
 * @file    temp_ctrl.c
 * @brief   温湿度控制模块:读 DHT11,温度高 → LED 流水,否则熄灭
 ******************************************************************************
 */
#include "stm32f4xx_hal.h"
#include "temp_ctrl.h"
#include "dht11.h"
#include "led.h"
#include "fan.h"
#include "OLED.h"
#include <stdio.h>
#include <string.h>

/* 模块内部状态 */
static float s_temp = 0.0f; /* 最近一次温度 */
static float s_humi = 0.0f; /* 最近一次湿度 */

/**
 * @brief  模块初始化(初始化 DHT11)
 */
void TempCtrl_Init(void)
{
    DHT11_Init();
}

/**
 * @brief  主循环任务
 *   每轮读取 DHT11:
 *   温度 > 阈值 → LED 流水灯
 *   温度 ≤ 阈值 → 全部熄灭
 * @note   读取失败时保持上次状态,避免误动作
 */
void TempCtrl_Task(void)
{
    char buf[32];

    if (DHT11_Read(&s_temp, &s_humi) == 1)
    {
        /* DHT11 读取成功 */
        LED_On(0);
        LED_Off(1);

        /* OLED 显示温湿度 */
        OLED_ClearArea(0, 0, 128, 16);
        snprintf(buf, sizeof(buf), "Temp: %.1f C", s_temp);
        OLED_ShowString(0, 0, buf, OLED_8X16);

        OLED_ClearArea(0, 16, 128, 16);
        snprintf(buf, sizeof(buf), "Humi: %.1f %%", s_humi);
        OLED_ShowString(0, 16, buf, OLED_8X16);

        /* 显示阈值和状态 */
        OLED_ClearArea(0, 32, 128, 16);
        if (s_temp > TEMP_HIGH_THRESHOLD)
        {
            snprintf(buf, sizeof(buf), "HOT! >%.0f", TEMP_HIGH_THRESHOLD);
        }
        else
        {
            snprintf(buf, sizeof(buf), "OK  <=%.0f", TEMP_HIGH_THRESHOLD);
        }
        OLED_ShowString(0, 32, buf, OLED_8X16);

        OLED_UpdateArea(0, 0, 128, 48);

        // 只有真正超过阈值才流水+风扇
        if (s_temp > TEMP_HIGH_THRESHOLD)
        {
            Fan_Reverse();
            LED_Waterfall(200);
        }
        else
        {
            Fan_Stop();
            LED_SetAll(LED_OFF);
        }
    }
    else
    {
        /* DHT11 读取失败 */
        LED_Off(0);
        LED_On(1);

        OLED_ClearArea(0, 0, 128, 16);
        OLED_ShowString(0, 0, "DHT11 Read Fail!", OLED_8X16);
        OLED_UpdateArea(0, 0, 128, 16);

        return;
    }
}

float TempCtrl_GetTemperature(void)
{
    return s_temp;
}

float TempCtrl_GetHumidity(void)
{
    return s_humi;
}
