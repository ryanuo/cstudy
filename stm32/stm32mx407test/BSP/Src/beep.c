
#include "beep.h"

#define BEEP_PIN GPIO_PIN_8
#define BEEP_PORT GPIOF

void BEEP_on(void) { HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_RESET); }

void BEEP_off(void) { HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_SET); }
