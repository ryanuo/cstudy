// LIGHT.c
#include "stm32f4xx.h"

void LIGHT_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	// ADC3 ≈‰÷√
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC3, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 1, ADC_SampleTime_480Cycles);
	ADC_Cmd(ADC3, ENABLE);
}

uint16_t LIGHT_GetValue(void)
{
	uint32_t timeout = 10000;
	ADC_SoftwareStartConv(ADC3);
	while (ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET)
	{
		if (--timeout == 0)
			return 0;
	}
	return ADC_GetConversionValue(ADC3);
}