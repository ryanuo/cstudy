#include "stm32f4xx.h"                  // Device header	

void LED_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	
	GPIO_Init(GPIOE,&GPIO_INSTRUCT);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	
	GPIO_Init(GPIOF,&GPIO_INSTRUCT);
	
	GPIO_SetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14);
	GPIO_SetBits(GPIOF,GPIO_Pin_9 | GPIO_Pin_10);
}

void LED3_off(void)
{
	GPIO_SetBits(GPIOE,GPIO_Pin_13);
}

void LED4_off(void)
{
	GPIO_SetBits(GPIOE,GPIO_Pin_14);
}

void LED1_off(void)
{
	GPIO_SetBits(GPIOF,GPIO_Pin_9);
}

void LED2_off(void)
{
	GPIO_SetBits(GPIOF,GPIO_Pin_10);
}

void LED3_on(void)
{
	GPIO_ResetBits(GPIOE,GPIO_Pin_13);
}

void LED4_on(void)
{
	GPIO_ResetBits(GPIOE,GPIO_Pin_14);
}

void LED1_on(void)
{
	GPIO_ResetBits(GPIOF,GPIO_Pin_9);
}

void LED2_on(void)
{
	GPIO_ResetBits(GPIOF,GPIO_Pin_10);
}
