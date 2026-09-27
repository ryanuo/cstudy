#include "stm32f4xx.h" // Device header
#include "EXTI.h"
#include "LED.h"

uint8_t flag = 0;
void EXTI_init(void)
{
	// 1）使能GPIOA和GPIOE的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	// 2）设置功能脚位--上拉输入
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_IN;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_0;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_INSTRUCT);
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	// 3）使GPIO配置生效
	GPIO_Init(GPIOE, &GPIO_INSTRUCT);
	// 4）使能SYSCFG时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	// 5）利用SYSCFG_EXTILineConfig将GPIO与EXTI连接起来
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource3);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);
	// 6）EXTI的结构体配置
	EXTI_InitTypeDef EXTI_INSTRUCT;
	EXTI_INSTRUCT.EXTI_Line = EXTI_Line0 | EXTI_Line2 | EXTI_Line3 | EXTI_Line4; // GPIO与EXTI对应
	EXTI_INSTRUCT.EXTI_LineCmd = ENABLE;										 // 开启
	EXTI_INSTRUCT.EXTI_Mode = EXTI_Mode_Interrupt;								 // 中断触发
	EXTI_INSTRUCT.EXTI_Trigger = EXTI_Trigger_Falling;							 // 下降沿触发
	// 7）使EXTI配置生效
	EXTI_Init(&EXTI_INSTRUCT);
	// 8）NVIC的结构体配置(NVIC分组)
	NVIC_InitTypeDef NVIC_INSTRUCT;
	NVIC_INSTRUCT.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级0-3
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority = 0;		 // 响应优先级0-3
	NVIC_Init(&NVIC_INSTRUCT);
	// 9）使NVIC配置生效

	NVIC_INSTRUCT.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级0-3
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority = 1;		 // 响应优先级0-3
	NVIC_Init(&NVIC_INSTRUCT);

	NVIC_INSTRUCT.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级0-3
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority = 2;		 // 响应优先级0-3
	NVIC_Init(&NVIC_INSTRUCT);

	NVIC_INSTRUCT.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级0-3
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority = 3;		 // 响应优先级0-3
	NVIC_Init(&NVIC_INSTRUCT);
}

// 10）启动外部中断的服务函数 IT获取中断标志位函数  非IT获取普通标志位函数
void EXTI0_IRQHandler(void) // SET触发 RESET不触发
{
	// 10.1）获取中断标志位  硬件置位1
	if (EXTI_GetITStatus(EXTI_Line0) == SET)
	{
		// 10.2）实际的业务代码
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
		{
		}
		// 10.3）清除标志位 软件复位0
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

// 10）启动外部中断的服务函数 IT获取中断标志位函数  非IT获取普通标志位函数
void EXTI2_IRQHandler(void) // SET触发 RESET不触发
{
	// 10.1）获取中断标志位  硬件置位1
	if (EXTI_GetITStatus(EXTI_Line2) == SET)
	{
		// 10.2）实际的业务代码
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) == 0)
		{
			LED2_on();
			flag = 2;
		}
		// 10.3）清除标志位 软件复位0
		EXTI_ClearITPendingBit(EXTI_Line2);
	}
}

// 10）启动外部中断的服务函数 IT获取中断标志位函数  非IT获取普通标志位函数
void EXTI3_IRQHandler(void) // SET触发 RESET不触发
{
	// 10.1）获取中断标志位  硬件置位1
	if (EXTI_GetITStatus(EXTI_Line3) == SET)
	{
		// 10.2）实际的业务代码
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) == 0)
		{
			flag = 3;
		}
		// 10.3）清除标志位 软件复位0
		EXTI_ClearITPendingBit(EXTI_Line3);
	}
}

// 10）启动外部中断的服务函数 IT获取中断标志位函数  非IT获取普通标志位函数
void EXTI4_IRQHandler(void) // SET触发 RESET不触发
{
	// 10.1）获取中断标志位  硬件置位1
	if (EXTI_GetITStatus(EXTI_Line4) == SET)
	{
		// 10.2）实际的业务代码
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == 0)
		{
			flag = 2;
		}
		// 10.3）清除标志位 软件复位0
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}
