#include "stm32f4xx_hal.h"
#include "light.h"
#include "led.h"
#include "beep.h"

#define LIGHT_PIN GPIO_PIN_7
#define LIGHT_PORT GPIOF

void Light_Init(void)
{
    // PB14 — 光敏传感器数字输入
    __HAL_RCC_GPIOF_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = LIGHT_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(LIGHT_PORT, &gpio);
}

uint16_t Light_GetValue(void)
{
    // 数字光敏：读取电平（LOW = 亮, HIGH = 暗）
    if (HAL_GPIO_ReadPin(LIGHT_PORT, LIGHT_PIN) == GPIO_PIN_RESET)
    {
        return 0; // 亮
    }
    return 4095; // 暗
}

void Light_Start(void)
{
    if (Light_GetValue() < THRESHOLD_DARK)
    {
        LED_SetAll(0);
        Beep_Off();
    }
    else
    {
        LED_SetAll(1);
        Beep_On();
    }
}
