#ifndef _BEEP_H_
#define _BEEP_H_
void BEEP_init(void);
void BEEP_on(void);
void BEEP_off(void);
void BEEP_TriggerNonBlocking(uint32_t duration_ms);
void BEEP_Task(void);
#endif