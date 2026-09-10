/**
 * @file    clock.h
 * @brief   HSE PLL / HSI PLL 运行时切换模块
 * @details 两种模式都是 168MHz SYSCLK，只是时钟源不同：
 *            HSE PLL —— 外部 8MHz 晶振（精度 ±10~30ppm）
 *            HSI PLL —— 内部 16MHz RC（精度 ±1%）
 *          切换后 PA8(MCO1) 输出 PLLCLK/5 = 33.6MHz，示波器可直接测出频率差。
 */

#ifndef __CLOCK_H
#define __CLOCK_H

#include "stdint.h"

/* 时钟模式 */
#define CLK_MODE_HSE_PLL    0U
#define CLK_MODE_HSI_PLL    1U

/**
 * @brief  初始化：配置 PA8(MCO1) 输出 + 默认进入 HSE PLL 168MHz
 */
void Clock_Init(void);

/**
 * @brief  切换 HSE PLL <-> HSI PLL
 */
void Clock_Toggle(void);

/**
 * @brief  获取当前模式
 * @retval CLK_MODE_HSE_PLL / CLK_MODE_HSI_PLL
 */
uint8_t Clock_GetMode(void);

/**
 * @brief  读出 MCO1 当前实际配置的输出频率
 * @note   分频系数和时钟源都从 RCC->CFGR 里读回来，改了 MCO1_Init() 的配置后
 *         屏幕会自动跟着变，不用再手改显示代码。
 *         注意：这是按寄存器算出的标称值，晶振/RC 的实际偏差它测不出来。
 * @retval 单位 Hz
 */
uint32_t Clock_GetMco1Freq(void);

#endif /* __CLOCK_H */
