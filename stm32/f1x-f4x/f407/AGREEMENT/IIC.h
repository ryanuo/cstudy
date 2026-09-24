#ifndef __IIC_H
#define __IIC_H

#include "stm32f4xx.h"

void IIC_init(void);
void IIC_setsdamode(GPIOMode_TypeDef mode);

#endif