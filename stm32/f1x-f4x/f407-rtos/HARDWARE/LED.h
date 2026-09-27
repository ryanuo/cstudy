#ifndef _LED_H_
#define _LED_H_
void LED_init(void);
void LED1_on(void);
void LED2_on(void);
void LED3_on(void);
void LED4_on(void);
void LED1_off(void);
void LED2_off(void);
void LED3_off(void);
void LED4_off(void);

// 流水灯
void    LED_FlowInit(void);
void    LED_FlowRun(void);
void    LED_FlowEnable(uint8_t en);
uint8_t LED_FlowIsEnabled(void);
void    LED_FlowSetInterval(uint32_t ms);   /* 可选，不要可删 */
#endif