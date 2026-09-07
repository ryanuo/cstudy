#include "light.h"
#include "led.h"
#include "stm32f1xx_hal.h"

#define LIGHT_PIN   GPIO_PIN_14
#define LIGHT_PORT  GPIOB

void Light_Init(void)
{
    // PB14 — 光敏传感器数字输入
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin  = LIGHT_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(LIGHT_PORT, &gpio);
}

uint16_t Light_GetValue(void)
{
    // 数字光敏：读取电平（LOW = 亮, HIGH = 暗）
    if (HAL_GPIO_ReadPin(LIGHT_PORT, LIGHT_PIN) == GPIO_PIN_RESET) {
        return 0;   // 亮
    }
    return 4095;    // 暗
}

void Light_AutoLED(void)
{
    uint16_t light = Light_GetValue();
    if (light < THRESHOLD_DARK) {
        LED_On(LED1);
        LED_Off(LED2);
        LED_On(LED3);   // 天亮 -> LED3 亮
    } else {
        LED_Off(LED1);
        LED_On(LED2);
        LED_Off(LED3);  // 天黑 -> LED3 灭
    }
}
