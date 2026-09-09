/**
 ******************************************************************************
 * @file    key.c
 * @brief   按键驱动(4 键,全部低电平有效,内部上拉)
 *
 *   KEY0 -> PA0   按下 = LOW
 *   KEY1 -> PE2   按下 = LOW
 *   KEY2 -> PE3   按下 = LOW
 *   KEY3 -> PE4   按下 = LOW
 ******************************************************************************
 */
#include "stm32f4xx_hal.h"
#include "key.h"
#include "led.h"
#include "fan.h"
#include "beep.h"

/* ---- 引脚定义 ---- */
#define KEY0_GPIO GPIOA
#define KEY0_PIN GPIO_PIN_0

#define KEY1_GPIO GPIOE
#define KEY1_PIN GPIO_PIN_2

#define KEY2_GPIO GPIOE
#define KEY2_PIN GPIO_PIN_3

#define KEY3_GPIO GPIOE
#define KEY3_PIN GPIO_PIN_4

/**
 * @brief  按键 GPIO 初始化:输入 + 内部上拉(空闲=高,按下=低)
 */
void Key_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;

    gpio.Pin = KEY0_PIN;
    HAL_GPIO_Init(KEY0_GPIO, &gpio);

    gpio.Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN;
    HAL_GPIO_Init(KEY1_GPIO, &gpio);
}

/**
 * @brief  读取单个按键电平(按下=1)
 */
static uint8_t Key_Pressed(GPIO_TypeDef *port, uint16_t pin)
{
    return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET) ? 1 : 0;
}

/**
 * @brief  非阻塞边沿检测:只在"松开→按下"瞬间返回一次键值
 * @retval KEY0_VAL~KEY3_VAL,无按键/按住不放/松开时返回 KEY_NONE
 */
uint8_t Key_GetNum(void)
{
    static uint8_t last0 = 0, last1 = 0, last2 = 0, last3 = 0;
    static uint8_t first_call = 1; /* 首次调用只同步状态,不触发 */

    uint8_t cur0 = Key_Pressed(KEY0_GPIO, KEY0_PIN);
    uint8_t cur1 = Key_Pressed(KEY1_GPIO, KEY1_PIN);
    uint8_t cur2 = Key_Pressed(KEY2_GPIO, KEY2_PIN);
    uint8_t cur3 = Key_Pressed(KEY3_GPIO, KEY3_PIN);
    uint8_t result = KEY_NONE;

    if (first_call)
    {
        /* 上电瞬间的引脚电平不作为"按下"事件,仅作为基准 */
        last0 = cur0;
        last1 = cur1;
        last2 = cur2;
        last3 = cur3;
        first_call = 0;
        return KEY_NONE;
    }

    if (cur0 && !last0)
        result = KEY0_VAL;
    else if (cur1 && !last1)
        result = KEY1_VAL;
    else if (cur2 && !last2)
        result = KEY2_VAL;
    else if (cur3 && !last3)
        result = KEY3_VAL;

    last0 = cur0;
    last1 = cur1;
    last2 = cur2;
    last3 = cur3;

    return result;
}

/**
 * @brief  按键功能分发
 *   KEY0 -> 切换 LED0
 *   KEY1 -> 切换 LED1
 *   KEY2 -> 切换 LED2
 *   KEY3 -> 切换 LED3
 */
void Key_Start(void)
{
    static uint8_t led_state[LED_NUM] = {0}; /* 记录每盏灯亮/灭 */
    static uint8_t fan_state = 0;
    static uint8_t beep_state = 0;
    // static uint8_t light_state = 0;

    uint8_t key = Key_GetNum();
    if (key == KEY_NONE)
        return;

    uint8_t idx = key - 1;
    if (idx < 0)
        return;
    if (idx < LED_NUM)
    {
        if (led_state[idx] == 0)
        {
            LED_On(idx);
            led_state[idx] = 1;
        }
        else
        {
            LED_Off(idx);
            led_state[idx] = 0;
        }

        if (idx == 0)
        {
            if (fan_state == 1)
            {
                Fan_Stop();
                fan_state = 0;
            }
            else
            {
                Fan_Forward();
                fan_state = 1;
            }
        }
        else if (idx == 2)
        {
            if (beep_state == 1)
            {
                Beep_Off();
                beep_state = 0;
            }
            else
            {
                Beep_On();
                beep_state = 1;
            }
        }
    }
}
