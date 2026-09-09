#ifndef __FAN_H
#define __FAN_H

#include <stdint.h>

void Fan_Init(void);
void Fan_Forward(void);
void Fan_Reverse(void);
void Fan_Stop(void);
void Fan_Forward_Reverse(void);
void Fan_Forward_Reverse_Start(void);
void Fan_Update(void);
void Fan_LightControl(void);

#endif
