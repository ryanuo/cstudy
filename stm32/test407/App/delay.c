/**
 * @file    delay.c
 * @brief   基于 DWT 周期计数器的阻塞延时 + 运行时间统计
 * @details
 *   DWT->CYCCNT 是 Cortex-M4 内核自带的周期计数器，每个 HCLK 周期 +1。
 *   它和 SysTick 完全独立，既不占用中断也不影响 HAL 的 1ms 时基
 *   （HAL_Delay() 依赖 SysTick 中断累加 uwTick，是中断式延时）。
 *
 *   延时按「周期数」计，所以实际时间 = 周期数 / 真实 HCLK。
 *   时钟源从 HSE 切到 HSI 时，真实频率偏差 ±1%，
 *   同样的延时会长/短约 1%，Delay_GetMs() 也会同步跑偏。
 */

#include "stm32f4xx_hal.h"
#include "delay.h"

static uint32_t s_cyc_per_us = 168U;        /* 每微秒的 HCLK 周期数 */
static uint32_t s_cyc_per_ms = 168000U;     /* 每毫秒的 HCLK 周期数 */

static uint32_t s_last_cyc = 0;             /* 上次采样到的 CYCCNT */
static uint32_t s_wrap     = 0;             /* CYCCNT 已回绕的次数 */

void Delay_Init(void)
{
    /* 使能 DWT 周期计数器 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* 按当前 SYSCLK 计算每微秒/每毫秒的周期数 */
    SystemCoreClockUpdate();
    s_cyc_per_us = SystemCoreClock / 1000000U;
    s_cyc_per_ms = SystemCoreClock / 1000U;

    s_last_cyc = 0;
    s_wrap     = 0;
}

void Delay_Us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * s_cyc_per_us;     /* 1s 以内不会溢出 32 位 */

    /* 无符号减法：CYCCNT 回绕（约 25.6s 一次）也能正确比较 */
    while ((DWT->CYCCNT - start) < ticks)
    {
        __NOP();
    }
}

void Delay_Ms(uint32_t ms)
{
    while (ms >= 1000U)
    {
        Delay_Us(1000000U);
        ms -= 1000U;
    }

    if (ms > 0U)
    {
        Delay_Us(ms * 1000U);
    }
}

uint32_t Delay_GetMs(void)
{
    uint32_t now = DWT->CYCCNT;

    /* CYCCNT 是 32 位，168MHz 下约 25.6 秒回绕一次 */
    if (now < s_last_cyc)
    {
        s_wrap++;
    }
    s_last_cyc = now;

    uint64_t cycles = ((uint64_t)s_wrap << 32) | (uint64_t)now;
    return (uint32_t)(cycles / s_cyc_per_ms);
}
