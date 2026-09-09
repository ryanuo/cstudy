#include "fan.h"
#include "stm32f1xx_hal.h"
#include "light.h"

#define FAN_IA_PIN GPIO_PIN_13
#define FAN_IB_PIN GPIO_PIN_12
#define FAN_PORT GPIOB

static uint8_t s_fan_auto = 0;
static uint8_t s_fan_step = 0;
static uint16_t s_fan_tick = 0;

void Fan_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = FAN_IA_PIN | FAN_IB_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(FAN_PORT, &gpio);
    HAL_GPIO_WritePin(FAN_PORT, FAN_IA_PIN, GPIO_PIN_RESET);   // IA = LOW
    HAL_GPIO_WritePin(FAN_PORT, FAN_IB_PIN, GPIO_PIN_RESET); // IB = LOW
}

void Fan_Forward(void)
{
    HAL_GPIO_WritePin(FAN_PORT, FAN_IA_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(FAN_PORT, FAN_IB_PIN, GPIO_PIN_RESET);
}

void Fan_Reverse(void)
{
    HAL_GPIO_WritePin(FAN_PORT, FAN_IA_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(FAN_PORT, FAN_IB_PIN, GPIO_PIN_SET);
}

void Fan_Stop(void)
{
    HAL_GPIO_WritePin(FAN_PORT, FAN_IA_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(FAN_PORT, FAN_IB_PIN, GPIO_PIN_RESET);
    s_fan_auto = 0;  // 停止自动模式
}

/* 非阻塞正反转：启动后由 Fan_Update() 自动运行 */
void Fan_Forward_Reverse_Start(void)
{
    s_fan_auto = 1;
    s_fan_step = 0;
    s_fan_tick = 0;
    Fan_Forward();
}

void Fan_Update(void)
{
    if (!s_fan_auto) return;
    s_fan_tick++;
    if (s_fan_step == 0) {
        // 正转 2s
        if (s_fan_tick >= 2000) { Fan_Stop(); s_fan_step = 1; s_fan_tick = 0; }
    } else if (s_fan_step == 1) {
        // 停止 0.5s
        if (s_fan_tick >= 500) { Fan_Reverse(); s_fan_step = 2; s_fan_tick = 0; }
    } else if (s_fan_step == 2) {
        // 反转 2s
        if (s_fan_tick >= 2000) { Fan_Stop(); s_fan_step = 3; s_fan_tick = 0; }
    } else {
        // 停止 0.5s 后回到正转
        if (s_fan_tick >= 500) { Fan_Forward(); s_fan_step = 0; s_fan_tick = 0; }
    }
}

/* 光控风扇：光照强时转，光照弱时停 */
void Fan_LightControl(void)
{
    uint16_t light = Light_GetValue();
    if (light < THRESHOLD_DARK) {
        Fan_Forward();   // 天亮 → 转
    } else {
        Fan_Stop();      // 天黑 → 停
    }
}