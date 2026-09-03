#include "key.h"
#include "stm32f1xx_hal.h"
#include "led.h"

#define KEY_PORT GPIOA
#define KEY_PIN GPIO_PIN_0

void Key_Init(void)
{
    // 1. 使能 GPIOB 时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 2. 配置引脚参数
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_11; // PB1 和 PB11
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;         // 普通输入模式
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;           // 下拉输入（按下接正电源=HIGH）

    // 3. 执行初始化
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

uint8_t Key_GetNum(void)
{
    uint8_t KeyNum = 0;

    // 检测 PB1（按键 1）
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) // 高电平=按下
    {
        HAL_Delay(20); // 按下消抖
        while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET)
            ;          // 等待松手
        HAL_Delay(20); // 松手消抖
        KeyNum = 1;
    }

    // 检测 PB11（按键 2）
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) // 高电平=按下
    {
        HAL_Delay(20); // 按下消抖
        while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET)
            ;          // 等待松手
        HAL_Delay(20); // 松手消抖
        KeyNum = 2;
    }

    return KeyNum;
}

void Key_led_toggle_init(void)
{
    uint8_t key = Key_GetNum();
    if (key == 2)
    {
        LED_Toggle(0);
    }
    else if (key == 1)
    {
        LED_Toggle(1);
    }
}