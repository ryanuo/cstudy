#include "stm32f4xx.h"                  // Device header
#include "DELAY.h"

void DELAY_ms(uint32_t xms)
{
	while(xms--)
	{
		SysTick->CTRL = 0; // Disable SysTick
		SysTick->LOAD = SystemCoreClock/1000; // Count from 255 to 0 (256 cycles)
		SysTick->VAL = 0; // Clear current value as well as count flag
		SysTick->CTRL= 5;//Enable SysTick timer with processor clock 
		while((SysTick->CTRL&(1<<16)) == 0);//168000--->0
		SysTick->CTRL = 0; // Disable SysTick
	}
}

void DELAY_us(uint32_t xus)
{
	while(xus--)
	{
		SysTick->CTRL = 0; // Disable SysTick
		SysTick->LOAD = SystemCoreClock/1000/1000; // Count from 255 to 0 (256 cycles)
		SysTick->VAL = 0; // Clear current value as well as count flag
		SysTick->CTRL= 5;//Enable SysTick timer with processor clock 
		while((SysTick->CTRL&(1<<16)) == 0);//168--->0
		SysTick->CTRL = 0; // Disable SysTick
	}
}
