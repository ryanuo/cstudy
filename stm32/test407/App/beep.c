#include "beep.h"
#include "stm32f4xx_hal.h"

#define BEEP_PIN GPIO_PIN_8
#define BEEP_PORT GPIOF

void Beep_Init(void)
{
    /* 1. 开启 GPIOA 时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* 2. 配置 GPIO */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = BEEP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(BEEP_PORT, &GPIO_InitStruct);

    /* 3. 初始化为关闭状态 */
    HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_RESET);
}

void Beep_On(void)
{
    HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_SET);
}

void Beep_Off(void)
{
    HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_RESET);
}

void Beep_Toggle(void)
{
    HAL_GPIO_TogglePin(BEEP_PORT, BEEP_PIN);
}