#include "stm32f4xx.h" // Device header
#include "esp8266.h"

void LED_init(void)
{
    // 1. 开启 GPIOE, GPIOF 的时钟 (保留你原来的)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    
    // 2. 【新增】开启 GPIOC 的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIO_INSTRUCT;
    GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
    GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;

    // 初始化 E 和 F (保留你原来的)
    GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_Init(GPIOE, &GPIO_INSTRUCT);

    GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_Init(GPIOF, &GPIO_INSTRUCT);

    // 3. 【新增】初始化 PC13
    GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &GPIO_INSTRUCT);

    // 4. 设置默认电平 (保留你原来的，同时把 PC13 也置高，默认熄灭)
    GPIO_SetBits(GPIOE, GPIO_Pin_13 | GPIO_Pin_14);
    GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
    
    // 【新增】默认给 PC13 高电平（如果是低电平点亮，高电平就是熄灭）
    GPIO_SetBits(GPIOC, GPIO_Pin_13); 
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
		LED4_on();
	}
}

uint8_t LED_FlowIsEnabled(void)
{
	return s_enable;
}