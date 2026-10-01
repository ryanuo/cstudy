#include "stm32f4xx.h"                  // Device header
#include "WWDG.h"

void WWDG_init(void)
{
//1、使能WWDG看门狗的时钟
RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG,ENABLE);
//2、设置WWDG预分频值
WWDG_SetPrescaler(WWDG_Prescaler_8);
//3、设置WWDG的窗口上限
WWDG_SetWindowValue(100);
//4、开启WWDG窗口看门狗
WWDG_Enable(120);
//5、开启WWDG提前唤醒中断EWI
WWDG_ClearFlag();
WWDG_EnableIT();
//6、NVIC的结构体配置
NVIC_InitTypeDef NVIC_INSTRUCT;
NVIC_INSTRUCT.NVIC_IRQChannel = WWDG_IRQn;
NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 3;
NVIC_INSTRUCT.NVIC_IRQChannelSubPriority = 3;
//7、使NVIC配置生效
NVIC_Init(&NVIC_INSTRUCT);
}

//8、提供WWDG的中断服务函数
void WWDG_IRQHandler(void)
{
 if(WWDG_GetFlagStatus() == SET)
 {
   WWDG_SetCounter(120);
	 WWDG_ClearFlag();
 }
}

