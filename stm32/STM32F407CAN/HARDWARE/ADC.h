#ifndef __ADC__H_
#define __ADC__H_
#include "stm32f4xx.h"                  // Device header
void ADC1PA5_init(void);
uint16_t ADC1PA5_getvalue(void);
extern volatile uint32_t ADC_convalue;//´æ´¢Æ÷
#endif
