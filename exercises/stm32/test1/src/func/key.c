#include "key.h"
#include "led.h"
#include "fan.h"
#include "buzzer.h"
#include "stm32f1xx_hal.h"

void Key_Init(void)
{
    // PA3/PA4/PA5 — 用户按键（接正电源，按下=HIGH）
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin  = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &gpio);
}

// 非阻塞边沿检测：只在按下的瞬间返回一次
uint8_t Key_GetNum(void)
{
    static uint8_t last_pin3 = 0, last_pin4 = 0, last_pin5 = 0;
    uint8_t cur_pin3 = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t cur_pin4 = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t cur_pin5 = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t result = 0;

    if (cur_pin3 && !last_pin3) result = 1;
    else if (cur_pin4 && !last_pin4) result = 2;
    else if (cur_pin5 && !last_pin5) result = 3;

    last_pin3 = cur_pin3;
    last_pin4 = cur_pin4;
    last_pin5 = cur_pin5;
    return result;
}

uint8_t Key_PA5_Pressed(void)
{
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) ? 1 : 0;
}

/*
 * 方案 A：独立开关
 * PA3：风扇正转/停止 切换
 * PA4：LED1 亮/灭 切换
 * PA5：蜂鸣器唱歌/暂停 切换
 */
void Key_led_toggle_init(void)
{
    static uint8_t fan_state = 0;    // 0=停止, 1=正转
    static uint8_t led1_state = 0;   // 0=灭, 1=亮

    uint8_t key = Key_GetNum();
    if (key == 0) return;

    if (key == 1) {
        // PA3：风扇正转/停止 切换
        if (fan_state) {
            Fan_Stop();
            fan_state = 0;
        } else {
            Fan_Forward();
            fan_state = 1;
        }
    } else if (key == 2) {
        // PA4：LED1 亮/灭 切换
        if (led1_state) {
            LED_Off(LED1);
            led1_state = 0;
        } else {
            LED_On(LED1);
            led1_state = 1;
        }
    } else if (key == 3) {
        // PA5：蜂鸣器唱歌/暂停 切换
        if (Music_IsPlaying()) {
            Music_Toggle();  // 暂停
        } else {
            Buzzer_Play(1);  // 播放《起风了》
        }
    }
}
