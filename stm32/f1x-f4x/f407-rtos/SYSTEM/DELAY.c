#include "DELAY.h"
#include "sys.h"

#include "FreeRTOS.h"
#include "task.h"

static u8 fac_us = 0;  // us延时倍乘数
static u16 fac_ms = 0; // ms延时倍乘数,在os下,代表每个节拍的ms数

// 初始化延迟函数
// 当使用FreeRTOS的时候,此函数会初始化FreeRTOS的时钟节拍
// SYSTICK的时钟固定为AHB时钟
// SYSCLK:系统时钟频率
void DELAY_init(u8 SYSCLK)
{
    fac_us = SYSCLK;                    // 给 DELAY_us 用
    fac_ms = 1000 / configTICK_RATE_HZ; // 给 DELAY_ms 用

#if (INCLUDE_xTaskGetSchedulerState == 0) // 裸机
    u32 reload;
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    reload = SYSCLK * (1000000 / configTICK_RATE_HZ);
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD = reload;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
#endif
}

// 延时nus
// nus为要延时的us数.
// 注意:nus的值,不要大于798915us(最大值即2^24/fac_us@fac_us=21)
void DELAY_us(u32 nus)
{
    u32 ticks;
    u32 told, tnow, tcnt = 0;
    u32 reload = SysTick->LOAD; // LOAD的值
    ticks = nus * fac_us;       // 需要的节拍数

    told = SysTick->VAL; // 刚进入时的计数器值
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow; // 这里注意一下SYSTICK是一个递减的计数器就可以了.
            else
                tcnt += reload - tnow + told;
            told = tnow;
            if (tcnt >= ticks)
                break; // 时间超过/等于要延迟的时间,则退出.
        }
    };
}
// 延时nms
// nms:要延时的ms数
// nms:0~65535
void DELAY_ms(u16 nms)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) // 如果OS已经在跑了,并且不是在中断里面(中断里面不能任务调度)
    {
        if (nms >= fac_ms) // 延时的时间大于OS的最少时间周期
        {
            vTaskDelay(nms / fac_ms); // OS延时
        }
        nms %= fac_ms; // OS已经无法提供这么小的延时了,采用普通方式延时
    }
    DELAY_us((u32)(nms * 1000)); // 普通方式延时
}
