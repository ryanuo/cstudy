#ifndef __SYS_H
#define __SYS_H

#include "stm32f4xx.h"

/* ---------- 位带操作核心宏 ---------- */
#define BITBAND(addr, bitnum)  ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5) + ((bitnum) << 2))
#define MEM_ADDR(addr)         (*(volatile unsigned long *)(addr))
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))

/* ---------- GPIO 基址映射（用 ## 拼接） ---------- */
/* ODR 偏移 +20，IDR 偏移 +16 */
#define GPIO_ODR_ADDR(x)  (GPIO##x##_BASE + 20)
#define GPIO_IDR_ADDR(x)  (GPIO##x##_BASE + 16)

/* ---------- 输出/输入 单 IO 操作 ---------- */
#define Pxout(x, n)  BIT_ADDR(GPIO_ODR_ADDR(x), n)   /* 输出 */
#define Pxin(x, n)   BIT_ADDR(GPIO_IDR_ADDR(x), n)   /* 输入  */

/* ---------- 兼容原来的 PAout/PBin 写法 ---------- */
#define PAout(n)  Pxout(A, n)
#define PAin(n)   Pxin(A, n)
#define PBout(n)  Pxout(B, n)
#define PBin(n)   Pxin(B, n)
#define PCout(n)  Pxout(C, n)
#define PCin(n)   Pxin(C, n)
#define PDout(n)  Pxout(D, n)
#define PDin(n)   Pxin(D, n)
#define PEout(n)  Pxout(E, n)
#define PEin(n)   Pxin(E, n)
#define PFout(n)  Pxout(F, n)
#define PFin(n)   Pxin(F, n)
#define PGout(n)  Pxout(G, n)
#define PGin(n)   Pxin(G, n)
#define PHout(n)  Pxout(H, n)
#define PHin(n)   Pxin(H, n)
#define PIout(n)  Pxout(I, n)
#define PIin(n)   Pxin(I, n)

/* ---------- 汇编函数 ---------- */
void WFI_SET(void);
void INTX_DISABLE(void);
void INTX_ENABLE(void);
void MSR_MSP(u32 addr);

#endif