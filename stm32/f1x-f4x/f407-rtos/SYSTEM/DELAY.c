#include "stm32f4xx.h"                  // Device header
#include "DELAY.h"

/* ---------------------------------------------------------------------------
 * 微秒/毫秒延时改用 DWT 周期计数器（CYCCNT），绝不再碰 SysTick。
 *
 * 为什么必须改：原来 DELAY_us/DELAY_ms 是靠 SysTick->CTRL = 0 / = 5 自己开关 SysTick
 * 来计时的 —— 写 5 时 TICKINT=0（不产生中断），循环结束时还留成 CTRL = 0。
 * 而本工程的 1ms 滴答（esp8266.c 里覆盖的 SysTick_Handler 里 esp_tick_ms++）是所有
 * ESP8266 超时/延时的唯一时间基准：DELAY_us 一跑，滴答就永久停摆，
 * WaitResponse 里 "while (tick - start < timeout)" 再也等不到超时边界 -> Web_Task 卡死。
 * 现场症状：网页点一次「读ID」（W25QXX_ReadID -> 软件 SPI，每个 bit 两次 DELAY_us），
 * 之后板子再也不回任何请求。
 *
 * DWT 只读 CPU 周期数，和 SysTick 中断互不干扰（DHT11.c 里用的是同一套做法）。
 * ------------------------------------------------------------------------- */

static uint32_t s_cycles_per_us = 0;

static void DELAY_DwtInit(void)
{
    if (s_cycles_per_us == 0)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        s_cycles_per_us = SystemCoreClock / 1000000U;   /* 168MHz -> 168 */
    }
}

void DELAY_us(uint32_t xus)
{
    uint32_t start, ticks;

    DELAY_DwtInit();
    ticks = xus * s_cycles_per_us;
    start = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - start) < ticks);
}

void DELAY_ms(uint32_t xms)
{
    while (xms--) DELAY_us(1000);
}
