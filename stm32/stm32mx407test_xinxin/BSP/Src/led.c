#include "led.h"
#include "board_pins.h"

typedef struct {
  GPIO_TypeDef *GPIOx;
  uint16_t GPIO_Pin;
} LED_Hardware_t;

/* 数量与 BOARD_LED_COUNT 强绑定：少写了编译期就报错，不会留下空表项 */
static const LED_Hardware_t led_hw[BOARD_LED_COUNT] = {
    {BOARD_LED_PORT, BOARD_LED0_PIN},
    {BOARD_LED_PORT, BOARD_LED1_PIN},
    {BOARD_LED_PORT, BOARD_LED2_PIN},
};

static uint8_t led_valid(uint8_t number) { return number < BOARD_LED_COUNT; }

void LED_On(uint8_t number) {
  if (!led_valid(number))
    return;
  HAL_GPIO_WritePin(led_hw[number].GPIOx, led_hw[number].GPIO_Pin,
                    BOARD_LED_ON_LEVEL ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LED_Off(uint8_t number) {
  if (!led_valid(number))
    return;
  HAL_GPIO_WritePin(led_hw[number].GPIOx, led_hw[number].GPIO_Pin,
                    BOARD_LED_ON_LEVEL ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void LED_Toggle(uint8_t number) {
  if (!led_valid(number))
    return;
  HAL_GPIO_TogglePin(led_hw[number].GPIOx, led_hw[number].GPIO_Pin);
}

void LED_Set(uint8_t number, uint8_t on) {
  if (on)
    LED_On(number);
  else
    LED_Off(number);
}
