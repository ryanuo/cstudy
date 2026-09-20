#include "stm32f4xx.h" // Device header
#include "esp8266.h"
#include <stdint.h>

void BEEP_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_8;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;

	GPIO_Init(GPIOF, &GPIO_INSTRUCT);

	GPIO_ResetBits(GPIOF, GPIO_Pin_8);
}

void BEEP_on(void)
{
	GPIO_SetBits(GPIOF, GPIO_Pin_8);
}

void BEEP_off(void)
{
	GPIO_ResetBits(GPIOF, GPIO_Pin_8);
}

static uint32_t beep_stop_tick = 0;
static uint8_t beep_is_active = 0;

/**
 * @brief  非阻塞触发蜂鸣器响一段指定时间
 * @param  duration_ms 响鸣持续时间（毫秒）
 */
void BEEP_TriggerNonBlocking(uint32_t duration_ms)
{
	BEEP_on();
	beep_stop_tick = ESP8266_GetTick() + duration_ms;
	beep_is_active = 1;
}

/**
 * @brief  蜂鸣器状态机轮询任务（需放入 main 的 while(1) 中持续执行）
 */
void BEEP_Task(void)
{
	if (!beep_is_active)
	{
		return;
	}

	/* 利用无符号数溢出回绕特性，安全判断超时 */
	if ((int32_t)(ESP8266_GetTick() - beep_stop_tick) >= 0)
	{
		BEEP_off();
		beep_is_active = 0;
	}
}
