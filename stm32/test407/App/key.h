#ifndef __KEY_H
#define __KEY_H

#include "stdint.h"

/* 键值定义 */
#define KEY_NONE   0
#define KEY0_VAL   1   /* PA0 */
#define KEY1_VAL   2   /* PE2 */
#define KEY2_VAL   3   /* PE3 */
#define KEY3_VAL   4   /* PE4 */

void Key_Init(void);
uint8_t Key_GetNum(void);      /* 非阻塞边沿检测:按下瞬间返回键值,否则 0 */
void Key_Start(void);          /* 按键功能分发:每键切换对应 LED */

#endif /* __KEY_H */
