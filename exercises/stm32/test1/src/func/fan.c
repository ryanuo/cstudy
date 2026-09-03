#include "fan.h"
#include "stm32f1xx_hal.h"
#include "light.h"

#define FAN_IA_PIN GPIO_PIN_13
#define FAN_IB_PIN GPIO_PIN_12
#define FAN_PORT GPIOB

void Fan_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = FAN_IA_PIN | FAN_IB_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(FAN_PORT, &gpio);
    HAL_GPIO_WritePin(FAN_PORT, FAN_IA_PIN, GPIO_PIN_RESET);   // IA = LOW
    HAL_GPIO_WritePin(FAN_PORT, FAN_IB_PIN, GPIO_PIN_RESET); // IB = LOW
}

void Fan_Forward(void)
{
    HAL_GPIO_WritePin(FAN_PORT, FAN_IA_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(FAN_PORT, FAN_IB_PIN, GPIO_PIN_RESET);
}

void Fan_Reverse(void)
{
    HAL_GPIO_WritePin(FAN_PORT, FAN_IA_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(FAN_PORT, FAN_IB_PIN, GPIO_PIN_SET);
}

void Fan_Stop(void)
{
    HAL_GPIO_WritePin(FAN_PORT, FAN_IA_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(FAN_PORT, FAN_IB_PIN, GPIO_PIN_RESET);
}

void Fan_Forward_Reverse(void)
{
    Fan_Forward();
    HAL_Delay(2000);
    Fan_Stop();
    HAL_Delay(500);
    Fan_Reverse();
    HAL_Delay(2000);
    Fan_Stop();
}

/* 光控风扇：光照强时转，光照弱时停 */
void Fan_LightControl(void)
{
    uint16_t light = Light_GetValue();
    if (light < THRESHOLD_DARK) {
        Fan_Forward();   // 天亮 → 转
    } else {
        Fan_Stop();      // 天黑 → 停
    }
}