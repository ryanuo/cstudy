#include "stm32f4xx.h"

/* ==================== DAC 配置 ==================== */
void DAC4_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    DAC_InitTypeDef DAC_InitStructure;

    /* 1. 使能时钟：GPIOA 在 AHB1，DAC 在 APB1 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    /* 2. PA4 配置为模拟模式（DAC 使能后自动连接输出） */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; /* F4 用 GPIO_Mode_AN，不是 GPIO_Mode_AIN */
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 3. 配置 DAC 通道 1 */
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None; /* 软件触发，写入即输出 */
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);

    /* 4. 使能 DAC 通道 1 */
    DAC_Cmd(DAC_Channel_1, ENABLE);
}

/* ==================== ADC 配置 ==================== */ 
void ADC6_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    /* 1. 使能时钟：GPIOA 在 AHB1，ADC1 在 APB2 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    /* 2. PA6 配置为模拟输入 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 3. ADC 通用参数配置（F4 特有，F1 没有这一步） */
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4; /* APB2/4 = 84/4 = 21MHz，低于 36MHz 上限 */
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    /* 4. ADC1 参数配置 */
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                               /* 单通道 */
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                         /* 单次转换 */
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; /* 软件触发 */
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;       /* 占位，未使用 */
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* 5. 配置 ADC1 通道 6（PA6），采样时间设为 480 周期 */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_480Cycles);

    /* 6. 使能 ADC1 */
    ADC_Cmd(ADC1, ENABLE);
}

void INOUT_Init(void)
{
    ADC6_Config();
    DAC4_Config();
}

/* ==================== 设置 DAC 输出电压 ==================== */
void DAC_SetVoltage(float voltage)
{
    uint16_t dac_value;

    if (voltage > 3.3f)
        voltage = 3.3f;
    if (voltage < 0.0f)
        voltage = 0.0f;

    /* 12 位右对齐：0 ~ 4095 对应 0 ~ VREF+ */
    dac_value = (uint16_t)((voltage / 3.3f) * 4095.0f);

    DAC_SetChannel1Data(DAC_Align_12b_R, dac_value);
}

/* ==================== 读取 ADC 电压值 ==================== */
float ADC_ReadVoltage(void)
{
    uint16_t adc_value;
    float voltage;

    /* 启动 ADC1 软件转换 */
    ADC_SoftwareStartConv(ADC1);

    /* 等待转换完成 */
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
        ;

    /* 读取 12 位 ADC 值 */
    adc_value = ADC_GetConversionValue(ADC1);

    voltage = (adc_value * 3.3f) / 4095.0f;

    return voltage;
}
