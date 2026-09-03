#include "buzzer.h"
#include "stm32f1xx_hal.h"

#define BUZZER_PIN  GPIO_PIN_1
#define BUZZER_PORT GPIOA

void Buzzer_Init(void) {
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = BUZZER_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUZZER_PORT, &gpio);
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET); // 初始不响
}

void Buzzer_On(void) {
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET); // 低电平响
}

void Buzzer_Off(void) {
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);   // 高电平停
}
