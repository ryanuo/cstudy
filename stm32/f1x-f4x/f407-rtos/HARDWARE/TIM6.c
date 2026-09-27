#include "stm32f4xx.h" // Device header
#include "TIM.h"
// 84MHZ/8400 = 10KHZ 0.1MS *5000= 500MS= 0.5S
void TIM6_init(void)
{
	// 1、使能定时器的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	// 2、定时器的结构体配置(时基单元)
	TIM_TimeBaseInitTypeDef TIM_INSTRUCT;
	TIM_INSTRUCT.TIM_ClockDivision = TIM_CKD_DIV1;	   // 不支持二级分频
	TIM_INSTRUCT.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
	TIM_INSTRUCT.TIM_Period = 10000 - 1;			   // 周期 0--65535
	TIM_INSTRUCT.TIM_Prescaler = 8400 - 1;			   // 预分频0--65535
	// 3、使定时器配置生效
	TIM_TimeBaseInit(TIM6, &TIM_INSTRUCT);
	// 4、开启定时器中断
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	// 5、NVIC结构体配置
	NVIC_InitTypeDef NVIC_INSTRUCT;
	NVIC_INSTRUCT.NVIC_IRQChannel = TIM6_DAC_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority = 1;
	// 6、使NVIC的配置生效
	NVIC_Init(&NVIC_INSTRUCT);
	// 7、运行定时器
	TIM_Cmd(TIM6, ENABLE);
}

// 8、提供定时器的中断服务
void TIM6_DAC_IRQHandler(void) // 500MS
{
	// 8.1、获取定时器中断标志
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		// 8.2、书写业务代码
		GPIO_ToggleBits(GPIOF, GPIO_Pin_10);
		// 8.3、清除中断标志位
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	}
}
