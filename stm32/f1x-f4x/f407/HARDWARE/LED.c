#include "stm32f4xx.h" // Device header
#include "esp8266.h"

void LED_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;

	GPIO_Init(GPIOE, &GPIO_INSTRUCT);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;

	GPIO_Init(GPIOF, &GPIO_INSTRUCT);

	GPIO_SetBits(GPIOE, GPIO_Pin_13 | GPIO_Pin_14);
	GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
}

void LED3_off(void)
{
	GPIO_SetBits(GPIOE, GPIO_Pin_13);
}

void LED4_off(void)
{
	GPIO_SetBits(GPIOE, GPIO_Pin_14);
}

void LED1_off(void)
{
	GPIO_SetBits(GPIOF, GPIO_Pin_9);
}

void LED2_off(void)
{
	GPIO_SetBits(GPIOF, GPIO_Pin_10);
}

void LED3_on(void)
{
	GPIO_ResetBits(GPIOE, GPIO_Pin_13);
}

void LED4_on(void)
{
	GPIO_ResetBits(GPIOE, GPIO_Pin_14);
}

void LED1_on(void)
{
	GPIO_ResetBits(GPIOF, GPIO_Pin_9);
}

void LED2_on(void)
{
	GPIO_ResetBits(GPIOF, GPIO_Pin_10);
}

/* ---------- 流水灯状态机 ---------- */
static uint8_t s_step = 0;
static uint32_t s_last = 0;
static uint8_t s_enable = 0; /* 默认关，等网页来开 */
static uint32_t s_interval = 200;

static const void (*s_on[4])(void) = {LED1_on, LED2_on, LED3_on, LED4_on};
static const void (*s_off[4])(void) = {LED1_off, LED2_off, LED3_off, LED4_off};

void LED_FlowInit(void)
{
	s_step = 0;
	s_last = ESP8266_GetTick();
	s_enable = 0;
	s_interval = 200;
	LED1_off();
	LED2_off();
	LED3_off();
	LED4_off();
}

void LED_FlowRun(void)
{
	if (!s_enable)
		return;
	if ((uint32_t)(ESP8266_GetTick() - s_last) < s_interval)
		return;
	s_last = ESP8266_GetTick();

	s_off[s_step](); /* 只灭上一个 */
	s_step = (s_step + 1) & 0x03;
	s_on[s_step](); /* 只亮下一个 */
}

/* ---------- 网页要用的接口 ---------- */

void LED_FlowEnable(uint8_t en)
{
	if (en)
	{
		s_enable = 1;
		s_last = ESP8266_GetTick(); /* 重新开时重置，避免立刻跳一步 */
	}
	else
	{
		s_enable = 0;
		LED1_off();
		LED2_off();
		LED3_off();
		LED4_off(); /* 关掉时全灭 */
	}
}

uint8_t LED_FlowIsEnabled(void)
{
	return s_enable;
}