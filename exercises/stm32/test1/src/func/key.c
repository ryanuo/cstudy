#include "key.h"
#include "stm32f1xx_hal.h"
#include "led.h"
#include "fan.h"

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

    // PA5 — 模式切换按键（接正电源，按下=HIGH）
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

uint8_t Key_PA5_Pressed(void)
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) // 高电平=按下
    {
        HAL_Delay(20); // 按下消抖
        while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET)
            ;          // 等待松手
        HAL_Delay(20); // 松手消抖
        return 1;
    }
    return 0;
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
    // static 保证函数退出后能够记住当前处于什么模式
    // 0: 停止, 1: 反转模式(LED_USER), 2: 正转模式(LED_BOARD)
    static uint8_t current_mode = 0;

    uint8_t key = Key_GetNum();
    if (key == 0) return; // 无按键按下直接返回

    // 1. 如果重复按下当前正在运行的模式 -> 全部关闭
    if (key == current_mode)
    {
        current_mode = 0;
        Fan_Stop();
        LED_Off(LED_BOARD);
        LED_Off(LED_USER);
    }
    // 2. 按下按键 2 -> 开启正转模式
    else if (key == 2)
    {
        current_mode = 2;
        LED_Off(LED_USER);   // 确保反转灯熄灭
        LED_On(LED_BOARD);   // 点亮正转灯
        Fan_Forward();       // 开启风扇正转
    }
    // 3. 按下按键 1 -> 开启反转模式
    else if (key == 1)
    {
        current_mode = 1;
        LED_Off(LED_BOARD);  // 确保正转灯熄灭
        LED_On(LED_USER);    // 点亮反转灯
        Fan_Reverse();       // 开启风扇反转
    }
}