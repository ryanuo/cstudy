#include "stm32f4xx.h"                  // Device header

void KEY_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_IN;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_0;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	
	GPIO_Init(GPIOA,&GPIO_INSTRUCT);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	
	GPIO_Init(GPIOE,&GPIO_INSTRUCT);
	
}

uint8_t KEY_getvalue(void)
{
	if(0 == GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0))
	{
		return 1;
	}
	else if(0 == GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2))
	{
		return 2;
	}
	else if(0 == GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3))
	{
		return 3;
	}
	else if(0 == GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4))
	{
		return 4;
	}
	else
		return 0;
}