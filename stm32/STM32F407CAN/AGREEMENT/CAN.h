#ifndef __CAN__H_
#define __CAN__H_
#include "stm32f4xx.h"                  // Device header
void CAN_init(void);
uint8_t CAN_sendmessage(uint16_t stdid,uint8_t *pdata,uint8_t length);
extern uint8_t CAN_rxbuff[8];
extern uint16_t CAN_rxid;
extern uint8_t CAN_rxlength;
extern uint8_t CAN_rxflag;
extern volatile uint32_t CAN_rx_irq_cnt;//收到帧就累加（不看 ID，任何帧都算）
#endif
