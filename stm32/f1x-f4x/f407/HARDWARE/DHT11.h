#ifndef _DHT11_H
#define _DHT11_H

#include "stm32f4xx.h"

/*
 * DHT11 温湿度（板载 U6 座，丝印 DHT11&DS18B20）
 *   数据脚 DATA = PG9（原理图网络名 DQ），板上已有 10K 上拉 R22/R23
 *   注意：同一个座也能插 DS18B20（单总线，协议不同）——换器件要换驱动
 *
 * 用法：
 *   DHT11_Init();                 开机初始化
 *   DHT11_Task();                 主循环里调：内部每 2 秒读一次（读一次约 25ms）
 *   DHT11_GetTemp()/DHT11_GetHumi()  取最近一次成功的值（整数 ℃ / %RH）
 */
void    DHT11_Init(void);
void    DHT11_Task(void);
uint8_t DHT11_Read(uint8_t *temp, uint8_t *humi);   /* 1 = 成功（手动读，Task 内部就是调它）*/
uint8_t DHT11_GetTemp(void);
uint8_t DHT11_GetHumi(void);

/* DHT11 数据第 2 字节 = 湿度小数、第 4 字节 = 温度小数（规格书标为保留、多数读出 0，
   但也有模块会给值）——把原始字节报出来，是 0 还是真小数一眼可见 */
uint8_t DHT11_GetTempDec(void);
uint8_t DHT11_GetHumiDec(void);
uint8_t DHT11_Ok(void);                             /* 最近一次读是否成功 */

#endif
