#include "led.h"

// 内部封装引脚硬件映射表
typedef struct {
    GPIO_TypeDef* port;
    uint16_t      pin;
} LED_HW_Config_t;

static const LED_HW_Config_t LED_TABLE[LED_COUNT] = {
    [LED_USER]  = { .port = GPIOA, .pin = GPIO_PIN_2 },
    [LED_BOARD] = { .port = GPIOC, .pin = GPIO_PIN_13 },
};

void LED_Init(void) {
    // 开启所需端口时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {
        .Mode  = GPIO_MODE_OUTPUT_PP,
        .Pull  = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

    for (int i = 0; i < LED_COUNT; i++) {
        gpio.Pin = LED_TABLE[i].pin;
        HAL_GPIO_Init(LED_TABLE[i].port, &gpio);
        // 初始高电平熄灭（低电平点亮模式）
        HAL_GPIO_WritePin(LED_TABLE[i].port, LED_TABLE[i].pin, GPIO_PIN_SET);
    }
}

void LED_On(LED_Id_t id) {
    if (id < LED_COUNT) {
        HAL_GPIO_WritePin(LED_TABLE[id].port, LED_TABLE[id].pin, GPIO_PIN_RESET);
    }
}

void LED_Off(LED_Id_t id) {
    if (id < LED_COUNT) {
        HAL_GPIO_WritePin(LED_TABLE[id].port, LED_TABLE[id].pin, GPIO_PIN_SET);
    }
}

void LED_Toggle(LED_Id_t id) {
    if (id < LED_COUNT) {
        HAL_GPIO_TogglePin(LED_TABLE[id].port, LED_TABLE[id].pin);
        HAL_Delay(100);
    }
}