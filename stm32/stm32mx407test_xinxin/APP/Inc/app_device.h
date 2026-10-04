#ifndef __APP_DEVICE_H
#define __APP_DEVICE_H

#include "main.h"

/* ============================================================
 *  业务层：设备物模型
 *
 *  - 处理云端下行（属性设置 LED/buzzer/fan、命令下发）→ 调 BSP 驱动
 *  - 组装上行内容（温湿度上报）
 *  - 工程里**唯一**使用 cJSON 的模块：通用层只搬字符串
 * ============================================================ */

/* 注册下行处理表 + 初始化可控外设（风扇引脚等） */
void Device_Init(void);

/* 读 DHT11 当前值并按物模型格式上报 */
void Device_ReportTempHumi(void);

#endif /* __APP_DEVICE_H */
