#ifndef __TEMP_CTRL_H
#define __TEMP_CTRL_H

/* 温度阈值:高于此值视为"温度高",触发流水灯 */
#define TEMP_HIGH_THRESHOLD   30.0f

void TempCtrl_Init(void);      /* 初始化 DHT11 */
void TempCtrl_Task(void);      /* 主循环任务:读温湿度,高温→流水灯,否则全灭 */

/* 读取当前值(可选,供其他模块显示用) */
float TempCtrl_GetTemperature(void);
float TempCtrl_GetHumidity(void);

#endif /* __TEMP_CTRL_H */
