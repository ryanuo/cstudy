#include "led.h"

typedef struct {
  GPIO_TypeDef *GPIOx;
  uint16_t GPIO_Pin;
} LED_Hardware_t;

static const LED_Hardware_t led_hw[4] = {
    {GPIOF, GPIO_PIN_9},
    {GPIOF, GPIO_PIN_10},
    {GPIOE, GPIO_PIN_13},
    {GPIOE, GPIO_PIN_14},
};

void LED_On(uint8_t number) {
  HAL_GPIO_WritePin(led_hw[number].GPIOx, led_hw[number].GPIO_Pin,
                    GPIO_PIN_RESET);
}

void LED_Off(uint8_t number) {
  HAL_GPIO_WritePin(led_hw[number].GPIOx, led_hw[number].GPIO_Pin,
                    GPIO_PIN_SET);
}

void LED_Toggle(uint8_t number) {
  HAL_GPIO_TogglePin(led_hw[number].GPIOx, led_hw[number].GPIO_Pin);
}