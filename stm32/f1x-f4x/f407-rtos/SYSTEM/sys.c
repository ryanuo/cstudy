#include "sys.h"

void WFI_SET(void)
{
	__asm__ volatile("wfi");
}

void INTX_DISABLE(void)
{
	__asm__ volatile("cpsid i" ::: "memory");
}

void INTX_ENABLE(void)
{
	__asm__ volatile("cpsie i" ::: "memory");
}

__attribute__((naked)) void MSR_MSP(u32 addr)
{
	__asm__ volatile("msr msp, r0\n"
					 "bx lr\n");
}