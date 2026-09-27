#ifndef __CAN_H
#define __CAN_H

#include "stm32f4xx.h"
void CAN_init();
uint8_t CAN_send_message(uint32_t id, uint8_t *data, uint8_t length);
#endif