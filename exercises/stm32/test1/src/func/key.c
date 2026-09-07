#include "key.h"
#include "led.h"
#include "fan.h"
#include "stm32f1xx_hal.h"

void Key_Init(void)
{
    // PA3/PA4/PA5 — 用户按键（接正电源，按下=HIGH）
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin  = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &gpio);

    // PB0 — 模式切换按键（接正电源，按下=HIGH）
    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpio.Pin  = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOB, &gpio);
}

uint8_t Key_GetNum(void)
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET) {
        HAL_Delay(20);
        while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET);
        HAL_Delay(20);
        return 1;
    }
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET) {
        HAL_Delay(20);
        while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET);
        HAL_Delay(20);
        return 2;
    }
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) {
        HAL_Delay(20);
        while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET);
        HAL_Delay(20);
        return 3;
    }
    return 0;
}

uint8_t Key_PA5_Pressed(void)
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) {
        HAL_Delay(20);
        while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET);
        HAL_Delay(20);
        return 1;
    }
    return 0;
}

uint8_t Key_Mode_Pressed(void)
{
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
        HAL_Delay(20);
        while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET);
        HAL_Delay(20);
        return 1;
    }
    return 0;
}

void Key_led_toggle_init(void)
{
    static uint8_t current_mode = 0;
    uint8_t key = Key_GetNum();

    if (key == 0) return;

    if (key == current_mode) {
        current_mode = 0;
        Fan_Stop();
        LED_Off(LED1);
        LED_Off(LED2);
    } else if (key == 2) {
        current_mode = 2;
        LED_Off(LED1);
        LED_On(LED2);
        Fan_Forward();
    } else if (key == 1) {
        current_mode = 1;
        LED_Off(LED2);
        LED_On(LED1);
        Fan_Reverse();
    }
}
