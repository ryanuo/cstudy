/**
 * @file    delay.h
 * @brief   阻塞式延时模块（DWT 周期计数器，不用 SysTick、不产生中断）
 */

#ifndef __DELAY_H
#define __DELAY_H

#include "stdint.h"

/**
 * @brief  使能 DWT 周期计数器，并按当前 SYSCLK 计算每微秒的周期数
 * @note   只需调用一次，必须在时钟配置完成后调用；同时把运行时间计数清零
 */
void Delay_Init(void);

/**
 * @brief  微秒级阻塞延时
 */
void Delay_Us(uint32_t us);

/**
 * @brief  毫秒级阻塞延时
 */
void Delay_Ms(uint32_t ms);

/**
 * @brief  上电（Delay_Init）以来经过的毫秒数
 * @note   按「标称频率」换算，所以时钟源不准时这个数就会跑偏：
 *         HSE ±30ppm 基本不漂，HSI ±1% 时 100 秒能差出 1 秒。
 *         必须周期调用（间隔远小于 CYCCNT 回绕的 25.6 秒）才能不漏计。
 */
uint32_t Delay_GetMs(void);

#endif /* __DELAY_H */
