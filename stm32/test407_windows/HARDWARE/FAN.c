#include "stm32f4xx.h"                  // Device header

void FAN_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	
	GPIO_Init(GPIOC,&GPIO_INSTRUCT);
	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
	GPIO_ResetBits(GPIOC,GPIO_Pin_7); 
}

void FAN_forwardrotation(void)
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_6 | GPIO_Pin_7); // 先全部拉低
    GPIO_SetBits(GPIOC, GPIO_Pin_6);                 // 再置正转
}

void FAN_reverserotation(void)
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_6 | GPIO_Pin_7); // 先全部拉低
    GPIO_SetBits(GPIOC, GPIO_Pin_7);                 // 再置反转
}

void FAN_off(void)
{
	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
	GPIO_ResetBits(GPIOC,GPIO_Pin_7);
}	