#include "stm32f4xx.h" // Device header
#include "FreeRTOS.h"
#include "KEY.h"

/* ============================================================
 * 按键初始化
 *   PA0 -> KEY0 (返回值 1)
 *   PE2 -> KEY1 (返回值 2)
 *   PE3 -> KEY2 (返回值 3)
 *   PE4 -> KEY3 (返回值 4)
 *   均为上拉输入，按键另一端接 GND
 * ============================================================ */
void KEY_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* 1. 使能 GPIOA、GPIOE 时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	/* 2. 配置 PA0 —— KEY0 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;		/* 输入模式 */
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;		/* 上拉，按键接 GND */
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;		/* 输入模式下无效，填默认值 */
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz; /* 输入模式下无效，填默认值 */
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* 3. 配置 PE2 / PE3 / PE4 —— KEY1/KEY2/KEY3 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
}

uint8_t KEY_getvalue(void)
{
	uint8_t val = 0;

	if (0 == GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
		val = 1;
	else if (0 == GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2))
		val = 2;
	else if (0 == GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3))
		val = 3;
	else if (0 == GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4))
		val = 4;

	if (val != 0)
	{
		vTaskDelay(pdMS_TO_TICKS(20)); // 等抖动过去
		// 再确认一次
		if (1 == val && 0 != GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
			val = 0;
		if (2 == val && 0 != GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2))
			val = 0;
		if (3 == val && 0 != GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3))
			val = 0;
		if (4 == val && 0 != GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4))
			val = 0;
	}
	return val;
}