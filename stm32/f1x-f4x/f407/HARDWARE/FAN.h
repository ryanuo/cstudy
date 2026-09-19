#ifndef _FAN_H_
#define _FAN_H_
void FAN_init(void);
void FAN_forwardrotation(void);
void FAN_reverserotation(void);
void FAN_off(void);

/* 读 PC6/PC7 输出位回推：bit1=PC6, bit0=PC7（给网页状态和 OLED 自检用）*/
uint8_t FAN_ReadPins(void);
#endif