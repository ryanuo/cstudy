#ifndef __DAC_H
#define __DAC_H
#include "stdint.h"

void DAC1_Init(void);
void DAC1_SetRawValue(uint16_t value);
void DAC1_SetVoltage(float voltage);
#endif