#ifndef __INOUT_H__
#define __INOUT_H__ 
void DAC4_Config(void);
void ADC6_Init(void);
void DAC_SetVoltage(float voltage);
float ADC_ReadVoltage(void);
void INOUT_Init(void);
#endif