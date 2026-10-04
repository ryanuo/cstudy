#include "beep.h"
#include "board_pins.h"

#define BEEP_PIN BOARD_BEEP_PIN
#define BEEP_PORT BOARD_BEEP_PORT

void BEEP_on(void) {
  HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN,
                    BOARD_BEEP_ON_LEVEL ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void BEEP_off(void) {
  HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN,
                    BOARD_BEEP_ON_LEVEL ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void BEEP_Set(uint8_t on) {
  if (on)
    BEEP_on();
  else
    BEEP_off();
}

uint8_t BEEP_IsOn(void) {
  GPIO_PinState st = HAL_GPIO_ReadPin(BEEP_PORT, BEEP_PIN);
  return (BOARD_BEEP_ON_LEVEL ? (st == GPIO_PIN_SET) : (st == GPIO_PIN_RESET)) ? 1 : 0;
}
