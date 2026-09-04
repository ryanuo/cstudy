#include "servo.h"
#include "stm32f1xx_hal.h"

/*
 * 舵机软件 PWM：PA6 输出（非阻塞）
 * 周期 20ms，高电平 1-3ms 对应 0-180 度
 *
 * Servo_Update() 每 1ms 在 SysTick 中断里调用（非阻塞）
 * 通过内部状态机控制引脚电平
 *
 * servo_pulse: 高电平持续时间，单位 ms (1-3)
 */

#define SERVO_GPIO_PORT GPIOA
#define SERVO_GPIO_PIN  GPIO_PIN_6

uint8_t servo_angle = 90;
static uint8_t servo_pulse = 2;   /* 高电平 ms 数，默认 2ms = 90度 */
static uint8_t servo_tick = 0;    /* 周期内 tick 计数，每 1ms 加 1 */

void Servo_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = SERVO_GPIO_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SERVO_GPIO_PORT, &gpio);

    HAL_GPIO_WritePin(SERVO_GPIO_PORT, SERVO_GPIO_PIN, GPIO_PIN_RESET);
}

void Servo_SetAngle(uint8_t angle)
{
    if (angle > 180) angle = 180;
    servo_angle = angle;
    /* 0-180 度 -> 1-3 ms 高电平 */
    servo_pulse = 1 + ((uint16_t)angle * 2 + 90) / 180;  // 1, 2, or 3
    /* 重置周期，确保下次立即生效 */
    servo_tick = 19;
}

/* 每 1ms 调用一次（SysTick 中断），非阻塞 */
void Servo_Update(void)
{
    servo_tick++;
    if (servo_tick >= 20) {
        /* 周期结束，重新开始，拉高 */
        servo_tick = 0;
        HAL_GPIO_WritePin(SERVO_GPIO_PORT, SERVO_GPIO_PIN, GPIO_PIN_SET);
    } else if (servo_tick >= servo_pulse) {
        /* 脉宽结束，拉低 */
        HAL_GPIO_WritePin(SERVO_GPIO_PORT, SERVO_GPIO_PIN, GPIO_PIN_RESET);
    }
}
