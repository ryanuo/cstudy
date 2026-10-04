#ifndef __BSP_DHT11_H
#define __BSP_DHT11_H

#include "main.h"

/* ================= 温湿度数据结构体 ================= */
typedef struct {
  uint8_t temp_int; /* 温度整数部分 */
  uint8_t temp_dec; /* 温度小数部分 */
  uint8_t humi_int; /* 湿度整数部分 */
  uint8_t humi_dec; /* 湿度小数部分 */
  uint8_t valid;    /* 数据有效标志：1=有效，0=无效 */
} DHT11_Data_t;

/* ================= 对外接口 ================= */
uint8_t DHT11_Read(uint8_t *temp, uint8_t *humi);

uint8_t DHT11_GetTemp(void);
uint8_t DHT11_GetTempDec(void);
uint8_t DHT11_GetHumi(void);
uint8_t DHT11_GetHumiDec(void);
uint8_t DHT11_Ok(void);
DHT11_Data_t DHT11_GetData(void);

void DHT11_Task(void);

#endif