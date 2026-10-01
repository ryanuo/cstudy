#include "stm32f4xx.h"                  // Device header
#include "LIGHT.h"

void LIGHT_init(void)
{
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_IN;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_7;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	
	GPIO_Init(GPIOF,&GPIO_INSTRUCT);
}

uint8_t LIGHT_getvalue(void)
{
		if(1 == GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_7))
		{
			return 1;
		}
		else
		{
		  return 0;
		}
}


