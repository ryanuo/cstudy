#ifndef __ADC_H
#define __ADC_H
uint16_t ADC1_getvalue(void);
void ADC1PA5_Init(void);
uint8_t LIGHTSENSOR_GetPercent(void);
extern volatile uint32_t ADC1ConvertedValue;
#endif