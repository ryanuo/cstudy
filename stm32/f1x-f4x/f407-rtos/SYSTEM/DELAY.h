#ifndef __DELAY_H
#define __DELAY_H
#include "sys.h"

void DELAY_init(u8 SYSCLK);
void DELAY_ms(u16 nms);
void DELAY_us(u32 nus);

#endif
