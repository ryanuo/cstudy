#include "sensor_web.h"
#include "web.h"
#include "ADC.h"
#include "LIGHTSENSOR.h"
#include "DHT11.h"

void Sensor_WebState(void)
{
    Web_StateAppend(
        "\"light\":%u,\"pot\":%u,\"temp\":%u,\"humi\":%u,\"tdec\":%u,\"hdec\":%u",
        (unsigned)LIGHT_GetValue(),
        (unsigned)ADC1ConvertedValue,
        (unsigned)DHT11_GetTemp(),
        (unsigned)DHT11_GetHumi(),
        (unsigned)DHT11_GetTempDec(),
        (unsigned)DHT11_GetHumiDec());
}

/* 传感器目前只上报，没有动作路由；保留接口方便以后加 */
uint8_t Sensor_WebRoute(uint8_t link, const char *path, char *req)
{
    (void)link; (void)path; (void)req;
    return 0;
}