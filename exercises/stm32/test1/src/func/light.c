#include "light.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_adc.h"

#define LIGHT_PIN        GPIO_PIN_0
#define LIGHT_PORT       GPIOA
#define LIGHT_ADC_CHANNEL ADC_CHANNEL_0

#define LED_PIN          GPIO_PIN_13
#define LED_PORT         GPIOC

#define THRESHOLD_DARK   2000  // 低于此值视为天黑

static ADC_HandleTypeDef hadc1;

void Light_Init(void) {
    // 使能 GPIOA 和 ADC1 时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();

    // PA0 模拟输入
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin  = LIGHT_PIN;
    gpio.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(LIGHT_PORT, &gpio);

    // ADC 配置
    hadc1.Instance                   = ADC1;
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode    = DISABLE;
    hadc1.Init.NbrOfConversion       = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    HAL_ADC_Init(&hadc1);

    ADC_ChannelConfTypeDef ch = {0};
    ch.Channel      = LIGHT_ADC_CHANNEL;
    ch.Rank         = ADC_REGULAR_RANK_1;
    ch.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &ch);
}

uint16_t Light_GetValue(void) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return val;
}

void Light_AutoLED(void) {
    uint16_t light = Light_GetValue();
    if (light < THRESHOLD_DARK) {
        // 天黑 → LED 亮（低电平点亮）
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
    } else {
        // 天亮 → LED 灭
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    }
}
