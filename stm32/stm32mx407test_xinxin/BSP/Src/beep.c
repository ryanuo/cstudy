
#include "beep.h"
#include "stm32f4xx_hal_gpio.h"

#define BEEP_PIN GPIO_PIN_7
#define BEEP_PORT GPIOG

void BEEP_on(void) { HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_SET); }

void BEEP_off(void) { HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_RESET); }
