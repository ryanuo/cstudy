#include "fan.h"
#include "stm32f1xx_hal.h"

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
    HAL_GPIO_WritePin(FAN_PORT, FAN_IA_PIN, GPIO_PIN_SET);   // IA = HIGH
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
    HAL_Delay(2000);  // 正转运行 2 秒
    Fan_Stop();
    HAL_Delay(500);   // 停机缓冲 0.5 秒，避免机械剧烈反冲和反电动势冲击芯片
    Fan_Reverse();
    HAL_Delay(2000);  // 反转运行 2 秒
    Fan_Stop();
}