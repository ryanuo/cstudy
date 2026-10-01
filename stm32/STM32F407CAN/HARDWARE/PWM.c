#include "stm32f4xx.h"                  // Device header
#include "PWM.h"

void PWM_init(void)
{
	//1、使能GPIOF的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	//2、GPIO模式(复用模式)
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_AF;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_6;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	//3、使GPIO配置生效
	GPIO_Init(GPIOA,&GPIO_INSTRUCT);
	//4、使能定时器14的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13,ENABLE);
	//5、GPIO复用为TIM14
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_TIM13);
	//6、定时器14时基单元配置
	TIM_TimeBaseInitTypeDef TIM_INSTRUCT;
	TIM_INSTRUCT.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_INSTRUCT.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_INSTRUCT.TIM_Period = 100-1;//10MS
	TIM_INSTRUCT.TIM_Prescaler = 8400-1;//10K
	//7、使定时器14配置生效
	TIM_TimeBaseInit(TIM13,&TIM_INSTRUCT);
	//8、PWM功能配置
	TIM_OCInitTypeDef TIM_OCINSTRUCT;
	TIM_OCINSTRUCT.TIM_OCMode = TIM_OCMode_PWM1;//PWM1模式
	TIM_OCINSTRUCT.TIM_OCPolarity = TIM_OCPolarity_High;//高电平有效
	TIM_OCINSTRUCT.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCINSTRUCT.TIM_Pulse = 98;//0-65535
	//9、PWM配置生效
	TIM_OC1Init(TIM13,&TIM_OCINSTRUCT);
	//10、启动定时器14
	TIM_Cmd(TIM13,ENABLE);
}



