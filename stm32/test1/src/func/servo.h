#ifndef __SERVO_H
#define __SERVO_H

#include <stdint.h>

extern uint8_t servo_angle;

void Servo_Init(void);
void Servo_SetAngle(uint8_t angle);  /* 0-180 度 */
void Servo_Update(void);             /* 每 1ms 调用一次（SysTick） */

#endif
