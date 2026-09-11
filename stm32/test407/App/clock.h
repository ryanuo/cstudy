/**
 * @file    clock.h
 * @brief   运行时切换系统时钟：HSE PLL 168MHz / HSI 直连 16MHz / HSE 直连 8MHz
 * @details 三种模式 SYSCLK 依次为 168MHz / 16MHz / 8MHz，
 *          配合 main.c 里「不随系统时钟补偿」的闪烁延时，
 *          可以让同一颗 LED 在三种模式下闪出 21 倍的速度差。
 *          PA8(MCO1) 输出随模式换源，示波器可直接量：
 *            HSE PLL -> PLLCLK/5 = 33.6MHz
 *            HSI 直连 -> HSI/1     = 16MHz
 *            HSE 直连 -> HSE/1     =  8MHz
 */

#ifndef __CLOCK_H
#define __CLOCK_H

#include "stdint.h"

/* 时钟模式（按 KEY0 循环切换） */
#define CLK_MODE_HSE_PLL     0U   /* 168MHz  HSE 8MHz 晶振 x PLL */
#define CLK_MODE_HSI_DIRECT  1U   /*  16MHz  PLL 关，HSI 直接做 SYSCLK */
#define CLK_MODE_HSE_DIRECT  2U   /*   8MHz  PLL 关，HSE 直接做 SYSCLK */
#define CLK_MODE_COUNT       3U

/**
 * @brief  初始化：配置 PA8(MCO1) 引脚 + 默认进入 HSE PLL 168MHz
 */
void Clock_Init(void);

/**
 * @brief  切到下一个模式（循环）：168MHz -> 16MHz -> 8MHz -> 168MHz
 */
void Clock_Toggle(void);

/**
 * @brief  切到指定模式
 * @param  mode CLK_MODE_HSE_PLL / CLK_MODE_HSI_DIRECT / CLK_MODE_HSE_DIRECT
 */
void Clock_SetMode(uint8_t mode);

/**
 * @brief  获取当前模式
 */
uint8_t Clock_GetMode(void);

/**
 * @brief  获取当前模式名（给 OLED 显示用）
 */
const char *Clock_GetModeName(void);

/**
 * @brief  读出 MCO1 当前实际配置的输出频率
 * @note   分频系数和时钟源都从 RCC->CFGR 里读回来，MCO1 换源/换分频后
 *         屏幕会自动跟着变，不用再手改显示代码。
 *         注意：这是按寄存器算出的标称值，晶振/RC 的实际偏差它测不出来。
 * @retval 单位 Hz
 */
uint32_t Clock_GetMco1Freq(void);

#endif /* __CLOCK_H */
