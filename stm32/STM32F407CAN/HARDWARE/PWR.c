#include "stm32f4xx.h"                  // Device header
#include "PWR.h"
#include "LED.h"
#include <stdio.h>

void PWR_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
}

void SLEEP_mode(void)
{
   printf("enter sleep mode\r\n");
	 LED1_on();
	 __WFI();//进入睡眠模式
	 LED1_off();
	 printf("exit sleep mode\r\n");
}

void STOP_mode(void)
{
	printf("enter stop mode\r\n");
	LED1_on();
	PWR_EnterSTOPMode(PWR_LowPowerRegulator_ON,PWR_STOPEntry_WFI);//停止退出
	SystemInit();
}

void STANDBY_mode(void)
{
	printf("enter standby mode\r\n");
	LED1_on();
	PWR_ClearFlag(PWR_FLAG_WU);
	PWR_WakeUpPinCmd(ENABLE);//PA0唤醒模式
	PWR_EnterSTANDBYMode();
}
