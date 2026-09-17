#include "stm32f4xx.h"
#include "ADC.h"

void ADC1PA5_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 使能外部时钟模块
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div6;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    // ADC配置
    ADC_InitTypeDef ADC_INSTRUCT;
    ADC_INSTRUCT.ADC_Resolution = ADC_Resolution_12b;
    ADC_INSTRUCT.ADC_ScanConvMode = DISABLE;
    ADC_INSTRUCT.ADC_ContinuousConvMode = DISABLE;
    ADC_INSTRUCT.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_INSTRUCT.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    ADC_INSTRUCT.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_INSTRUCT.ADC_NbrOfConversion = 1;
    ADC_Init(ADC1, &ADC_INSTRUCT);

    // ADC 转换
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_3Cycles);
    ADC_Cmd(ADC1, ENABLE);
}

uint16_t ADC1_getvalue(void)
{
    uint32_t timeout = 10000;

    ADC_SoftwareStartConv(ADC1);

    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
    {
        if (--timeout == 0)
        {
            return 0; // 超时退出，可根据需求定义错误码
        }
    }

    return ADC_GetConversionValue(ADC1);
}