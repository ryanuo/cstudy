#include "led.h"
#include "stm32f1xx_hal.h"

#define LED_PIN      GPIO_PIN_10
#define LED_PORT     GPIOA
#define BOARD_LED_PIN  GPIO_PIN_13
#define BOARD_LED_PORT GPIOC

void LED_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = LED_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &gpio);
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); // 初始灭
}

void LED_Toggle(void) {
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}

void LED_On(void) {
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET); // 低电平点亮
}

void LED_Off(void) {
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

void BoardLED_Init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = BOARD_LED_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BOARD_LED_PORT, &gpio);
    HAL_GPIO_WritePin(BOARD_LED_PORT, BOARD_LED_PIN, GPIO_PIN_SET); // 初始灭
}

void BoardLED_Toggle(void) {
    HAL_GPIO_TogglePin(BOARD_LED_PORT, BOARD_LED_PIN);
}

void BoardLED_On(void) {
    HAL_GPIO_WritePin(BOARD_LED_PORT, BOARD_LED_PIN, GPIO_PIN_RESET);
}

void BoardLED_Off(void) {
    HAL_GPIO_WritePin(BOARD_LED_PORT, BOARD_LED_PIN, GPIO_PIN_SET);
}
