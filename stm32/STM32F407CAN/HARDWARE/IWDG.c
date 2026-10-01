#include "stm32f4xx.h"                  // Device header
#include "IWDG.h"

void IWDG_init(void)
{
//1、使能独立看门狗的时钟
IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
//2、设置独立看门狗的分频值
IWDG_SetPrescaler(IWDG_Prescaler_32);
//3、设置独立看门狗的计数值
IWDG_SetReload(1000);
//4、喂狗(重载计数器)
IWDG_ReloadCounter();
//5、开启看门狗
IWDG_Enable();
}


