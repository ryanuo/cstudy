#include "fan.h"

#define FAN_PIN0 GPIO_PIN_6
#define FAN_PIN1 GPIO_PIN_7

void FAN_forwardrotation(void) {
  HAL_GPIO_WritePin(GPIOC, FAN_PIN0 | FAN_PIN1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, FAN_PIN0, GPIO_PIN_SET);
}

void FAN_reverserotation(void) {
  HAL_GPIO_WritePin(GPIOC, FAN_PIN0 | FAN_PIN1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, FAN_PIN1, GPIO_PIN_SET);
}

void FAN_off(void) {
  HAL_GPIO_WritePin(GPIOC, FAN_PIN0, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, FAN_PIN1, GPIO_PIN_RESET);
}