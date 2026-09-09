/**
  ******************************************************************************
  * @file    led.c
  * @brief   LED 驱动模块实现
  ******************************************************************************
  */

#include "stm32f4xx_hal.h"
#include "led.h"

/* LED 硬件定义: GPIO 端口 + 引脚 */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t     pin;
} LED_Hardware_t;

static const LED_Hardware_t led_hw[LED_NUM] = {
    { GPIOF, GPIO_PIN_9  },  /* LED0 -> PF9  */
    { GPIOF, GPIO_PIN_10 },  /* LED1 -> PF10 */
    { GPIOE, GPIO_PIN_13 },  /* LED2 -> PE13 */
    { GPIOE, GPIO_PIN_14 },  /* LED3 -> PE14 */
};

/**
  * @brief  初始化所有 LED 对应的 GPIO(推挽输出,低电平点亮)
  */
void LED_Init(void)
{
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    for (uint8_t i = 0; i < LED_NUM; i++) {
        gpio.Pin = led_hw[i].pin;
        HAL_GPIO_Init(led_hw[i].port, &gpio);
        /* 初始全部熄灭(高电平) */
        HAL_GPIO_WritePin(led_hw[i].port, led_hw[i].pin, GPIO_PIN_SET);
    }
}

/**
  * @brief  点亮指定 LED(拉低)
  */
void LED_On(uint8_t idx)
{
    if (idx < LED_NUM) {
        HAL_GPIO_WritePin(led_hw[idx].port, led_hw[idx].pin, GPIO_PIN_RESET);
    }
}

/**
  * @brief  熄灭指定 LED(拉高)
  */
void LED_Off(uint8_t idx)
{
    if (idx < LED_NUM) {
        HAL_GPIO_WritePin(led_hw[idx].port, led_hw[idx].pin, GPIO_PIN_SET);
    }
}

/**
  * @brief  翻转指定 LED 状态
  */
void LED_Toggle(uint8_t idx)
{
    if (idx < LED_NUM) {
        HAL_GPIO_TogglePin(led_hw[idx].port, led_hw[idx].pin);
    }
}

/**
  * @brief  同时设置所有 LED 状态
  * @param  state: LED_ON 或 LED_OFF
  */
void LED_SetAll(uint8_t state)
{
    GPIO_PinState pin_state = (state == LED_ON) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    for (uint8_t i = 0; i < LED_NUM; i++) {
        HAL_GPIO_WritePin(led_hw[i].port, led_hw[i].pin, pin_state);
    }
}

/**
  * @brief  流水灯效果(阻塞式,在 while 循环中调用)
  * @param  interval_ms: 每盏灯停留时间(毫秒)
  */
void LED_Waterfall(uint16_t interval_ms)
{
    static uint8_t idx = 0;

    /* 全部熄灭 */
    LED_SetAll(LED_OFF);
    /* 点亮当前那个 */
    LED_On(idx);
    HAL_Delay(interval_ms);
    idx = (idx + 1) % LED_NUM;
}
