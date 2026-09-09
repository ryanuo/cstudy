#ifndef __DHT11_H
#define __DHT11_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 DHT11
 */
void DHT11_Init(void);

/**
 * @brief 读取 DHT11 温湿度
 *
 * @param temperature 温度，单位：℃
 * @param humidity    湿度，单位：%RH
 *
 * @return 1 = 成功
 *         0 = 失败
 */
uint8_t DHT11_Read(float *temperature, float *humidity);

#ifdef __cplusplus
}
#endif

#endif