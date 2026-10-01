#ifndef __IIC__H_
#define __IIC__H_
#include "stm32f4xx.h"                  // Device header
void IICSOFTWARE_init(void);
void IIC_start(void);
void IIC_stop(void);
void IIC_sendbyte(uint8_t byte);
uint8_t IIC_waitack(void);
uint8_t IIC_revicebyte(void);
void IIC_sendack(uint8_t ack);
#endif
