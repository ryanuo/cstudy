#include "fan.h"
#include "board_pins.h"

#define FAN_PORT BOARD_FAN_PORT
#define FAN_PIN0 BOARD_FAN_PIN0
#define FAN_PIN1 BOARD_FAN_PIN1

/* H 桥两根方向线：先都拉掉再拉其中一根，避免上下桥臂直通。
 * 引脚的时钟/模式由 CubeMX 生成的 MX_GPIO_Init() 负责配置，这里只写电平。 */
static void fan_write(uint8_t pin0_level, uint8_t pin1_level) {
  HAL_GPIO_WritePin(FAN_PORT, FAN_PIN0 | FAN_PIN1, GPIO_PIN_RESET);
  if (pin0_level)
    HAL_GPIO_WritePin(FAN_PORT, FAN_PIN0, GPIO_PIN_SET);
  if (pin1_level)
    HAL_GPIO_WritePin(FAN_PORT, FAN_PIN1, GPIO_PIN_SET);
}

void FAN_forwardrotation(void) {
  fan_write(BOARD_FAN_ON_LEVEL, !BOARD_FAN_ON_LEVEL);
}

void FAN_reverserotation(void) {
  fan_write(!BOARD_FAN_ON_LEVEL, BOARD_FAN_ON_LEVEL);
}

void FAN_off(void) { fan_write(0, 0); }

void FAN_Set(uint8_t on) {
  if (on)
    FAN_forwardrotation();
  else
    FAN_off();
}
