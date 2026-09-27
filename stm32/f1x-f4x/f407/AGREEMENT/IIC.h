#ifndef __IIC_H
#define __IIC_H

#include "stm32f4xx.h"
#include <stdint.h>

#define IIC_ACK     0
#define IIC_NACK    1

void IIC_Init(void);

void IIC_Start(void);
void IIC_Stop(void);

void IIC_SendByte(uint8_t byte);
uint8_t IIC_WaitAck(void);

uint8_t IIC_ReceiveByte(void);
void IIC_SendAck(uint8_t ack);

#endif