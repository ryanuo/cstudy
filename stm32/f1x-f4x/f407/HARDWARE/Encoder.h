#ifndef __ENCODER_H
#define __ENCODER_H
#include "stm32f4xx.h"

void Encoder_Init(void);
int32_t Encoder_GetCount(void);
uint8_t Encoder_GetSW(void);
void Encoder_ResetCount(void);
void Encoder_ClearSW(void);

#endif