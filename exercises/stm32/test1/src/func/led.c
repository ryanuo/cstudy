#include "led.h"

// LED 硬件映射表
typedef struct {
    GPIO_TypeDef* port;
    uint16_t      pin;
} LED_HW_Config_t;

static const LED_HW_Config_t LED_TABLE[LED_COUNT] = {
    [LED1]  = { .port = GPIOA, .pin = GPIO_PIN_0 },
    [LED2]  = { .port = GPIOA, .pin = GPIO_PIN_1 },
    [LED3]  = { .port = GPIOA, .pin = GPIO_PIN_2 },
};

void LED_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {
        .Mode  = GPIO_MODE_OUTPUT_PP,
        .Pull  = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

    for (int i = 0; i < LED_COUNT; i++) {
        gpio.Pin = LED_TABLE[i].pin;
        HAL_GPIO_Init(LED_TABLE[i].port, &gpio);
        HAL_GPIO_WritePin(LED_TABLE[i].port, LED_TABLE[i].pin, GPIO_PIN_SET);  // 初始高电平 = 灭
    }
}

void LED_On(LED_Id_t id)
{
    if (id < LED_COUNT) {
        HAL_GPIO_WritePin(LED_TABLE[id].port, LED_TABLE[id].pin, GPIO_PIN_RESET);  // 低电平点亮
    }
}

void LED_Off(LED_Id_t id)
{
    if (id < LED_COUNT) {
        HAL_GPIO_WritePin(LED_TABLE[id].port, LED_TABLE[id].pin, GPIO_PIN_SET);   // 高电平熄灭
    }
}

void LED_Toggle(LED_Id_t id)
{
    if (id < LED_COUNT) {
        HAL_GPIO_TogglePin(LED_TABLE[id].port, LED_TABLE[id].pin);
    }
}

/* 流水灯和闪烁：由 LED_Update() 每 1ms 调用 */
static uint8_t s_led_mode = 0;  // 1=LED1, 2=LED2, 3=LED3, 4=流水灯, 5=闪烁
static uint16_t s_led_tick = 0;
static uint8_t s_led_step = 0;

void LED_SetMode(uint8_t mode)
{
    s_led_mode = mode;
    s_led_tick = 0;
    s_led_step = 0;
    LED_Off(LED1);
    LED_Off(LED2);
    LED_Off(LED3);
    if (mode == 1)      LED_On(LED1);
    else if (mode == 2) LED_On(LED2);
    else if (mode == 3) LED_On(LED3);
}

void LED_Update(void)
{
    if (s_led_mode == 4) {
        // 流水灯：每 200ms 切换
        s_led_tick++;
        if (s_led_tick >= 200) {
            s_led_tick = 0;
            LED_Off(LED1); LED_Off(LED2); LED_Off(LED3);
            if (s_led_step == 0)      { LED_On(LED1); s_led_step = 1; }
            else if (s_led_step == 1) { LED_On(LED2); s_led_step = 2; }
            else if (s_led_step == 2) { LED_On(LED3); s_led_step = 0; }
        }
    } else if (s_led_mode == 5) {
        // 闪烁：每 500ms 翻转所有 LED
        s_led_tick++;
        if (s_led_tick >= 500) {
            s_led_tick = 0;
            LED_Toggle(LED1); LED_Toggle(LED2); LED_Toggle(LED3);
        }
    }
}
