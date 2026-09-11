#include "stm32f4xx.h" // Device header
#include "LED.h"
#include "BEEP.h"
#include "KEY.h"
#include "LIGHTSENSOR.h"
#include "FAN.h"
#include "DELAY.h"
#include "TIM.h"
#include "OLED.h"
// #include "CountSensor.h"
#include "Encoder.h"

uint8_t data = 0;
int main(void)
{
	// SysTick_Config(SystemCoreClock/1000/1000);
	LED_init();
	// BEEP_init();
	// KEY_init();
	// LIGHTSENSOR_init();
	// FAN_init();
	// TIM6_init();
	OLED_Init();
	// CountSensor_Init();
	Encoder_Init();

	// OLED_ShowString(0, 0, "Hello World", OLED_8X16); // ? OLED_6X8
	// OLED_ShowString(0, 16, "count:", OLED_6X8);		 // ? OLED_6X8

	// RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	// GPIO_InitTypeDef GPIO_INSTRUCT;			  // int a
	// GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;  // 输出
	// GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP; // 推挽
	// GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_6;
	// GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;	  // 输出模式下输入无效
	// GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz; // GPIO反转速率

	// GPIO_Init(GPIOC, &GPIO_INSTRUCT);

	int32_t last = 0;
	while (1)
	{
		// OLED_ShowNum(6 * 7, 16, CountSensor_GetValue(), 6, OLED_6X8);
		int32_t now = Encoder_GetCount();

		if (now != last)
		{
			last = now;
			OLED_ShowString(0, 16, "Enc:", OLED_6X8);
			OLED_ShowSignedNum(24, 16, now, 6, OLED_6X8);
			OLED_Update();
		}

		if (Encoder_GetSW())
		{
			// 如果是旋转误碰或毛刺，此时电平早已弹回高电平（1）
			// 只有用户真正按住按键时，电平才是稳定的低电平（0）
			if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_13) == 0)
			{
				LED1_on();
				Encoder_ResetCount();
			}
			Encoder_ClearSW(); // 无论真按还是误触，清除标志
		}

		/* ---- 诊断：A/B 相原始电平。缓慢转一格应看到 (A,B) 走 11→01→00→10 或 11→10→00→01 ---- */
		OLED_ShowString(0,  0, "A:", OLED_6X8);
		OLED_ShowNum(14, 0, GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5),  1, OLED_6X8);
		OLED_ShowString(36, 0, "B:", OLED_6X8);
		OLED_ShowNum(50, 0, GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_6), 1, OLED_6X8);

		OLED_Update();

		// LED1_on();

		// BEEP_on();
		// GPIO_SetBits(GPIOC,GPIO_Pin_6);
		// DELAY_ms(100);
		// LED1_off();
		// BEEP_off();
		// GPIO_ResetBits(GPIOC,GPIO_Pin_6);
		// DELAY_ms(100);

		/* Select the main PLL as system clock source */

		//	uint8_t data = KEY_getvalue();

		// if (data == 1)
		// {

		// 	LED1_on();
		// 	static volatile uint32_t counter = 0;
		// 	if (counter++ >= 1000000)
		// 	{
		// 		GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
		// 		counter = 0;
		// 	}
		// 	// DELAY_ms(1000);
		// 	LED1_off();
		// 	// DELAY_ms(1000);

		// 	/*RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
		// 	RCC->CFGR |= RCC_CFGR_SW_HSI;*/
		// }
		// if (data == 2)
		// {
		// 	LED1_off();
		// 	/*RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
		// 	RCC->CFGR |= RCC_CFGR_SW_HSE;*/
		// }
		// if (data == 3)
		// {
		// 	/*RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
		// 	RCC->CFGR |= RCC_CFGR_SW_PLL;*/
		// }
		/*uint8_t FAN_data = KEY_getvalue();
			if(FAN_data == 1)
		{
				FAN_forwardrotation();
			}
			else if(FAN_data == 2)
			{
				FAN_reverserotation();
			}
			else if(FAN_data == 3)
			{
				FAN_off();
			}

			uint8_t LIGHT_data = LIGHTSENSOR_getvalue();
			if(LIGHT_data == 1)
			{
				LED1_off();
				LED2_off();
				LED3_off();
				LED4_off();
			}
			else
			{
				LED1_on();
				LED2_on();
				LED3_on();
				LED4_on();
			}*/
	}
}

// void SysTick_Handler(void)
//{
//	data = KEY_getvalue();
//	/*static  volatile uint32_t counter =0;
//	if(counter++ >= 1000000)
//	{
//		GPIO_ToggleBits(GPIOF,GPIO_Pin_9);
//		counter = 0;
//	}	*/
// }