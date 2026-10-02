#include "count_sensor.h"

#define COUNT_PIN GPIO_PIN_8

static volatile uint32_t s_count = 0;

void CountSensor_Init(void) { s_count = 0; }

uint32_t CountSensor_GetValue(void) { return s_count; }

void CountSensor_Reset(void) { s_count = 0; }

void CountSensor_EXTI_Callback(uint16_t GPIO_Pin) {
  static uint32_t last_tick = 0;

  if (GPIO_Pin == GPIO_PIN_8) {
    uint32_t now = HAL_GetTick();
    if (now - last_tick > 20) // 20ms 内只算一次
    {
      s_count++;
      last_tick = now;
    }
  }
}