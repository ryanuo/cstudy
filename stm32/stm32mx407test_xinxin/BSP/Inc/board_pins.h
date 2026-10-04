#ifndef __BOARD_PINS_H
#define __BOARD_PINS_H

#include "main.h"

/* ============================================================
 *  板级引脚 / 有效电平 —— 唯一真值源
 *
 *  规范：
 *    - 所有外设的端口/引脚/有效电平只在本文件定义，驱动（led.c/beep.c/
 *      fan.c/bsp_dht11.c）和业务层都引用这里，别的地方不许再写 GPIOx。
 *    - 查出真值只改本文件，其余代码一行不动。
 *
 *  ★ 标记的行 = 需要你对着原理图核对后填的（当前值是"改之前的代码里写的"）
 * ============================================================ */

/* ---------------- LED（低电平点亮） ----------------
 * 来源：BSP/Src/led.c 原定义（PE3 / PE4 / PG9，均为 .ioc 里已配的推挽输出）*/
#define BOARD_LED_COUNT 3
#define BOARD_LED_PORT_E GPIOE
#define BOARD_LED_PORT_G GPIOG
#define BOARD_LED0_PIN GPIO_PIN_3
#define BOARD_LED1_PIN GPIO_PIN_4
#define BOARD_LED2_PIN GPIO_PIN_9
#define BOARD_LED_ON_LEVEL 0 /* 0 = 低电平点亮 */

/* ---------------- 蜂鸣器（PG7，高电平响） ----------------
 * ★ 注意：原来的 APP/Src/app_onenet.c 把 LED 写成 PG7、蜂鸣器写成 PG9，
 *   与 led.c / beep.c 正好相反。这里以驱动层为准，请核对是否与你板子一致。*/
#define BOARD_BEEP_PORT GPIOG
#define BOARD_BEEP_PIN GPIO_PIN_7
#define BOARD_BEEP_ON_LEVEL 1 /* 1 = 高电平响 */

/* ---------------- 风扇（H 桥方向线） ----------------
 * ★ 待确认：当前值取自 BSP/Src/fan.c（PC6/PC7），与 app_onenet.c 里写的
 *   PD0（= FSMC_D2，LCD 数据线！）冲突。真值查到后在 CubeMX 里把这两个脚
 *   配成推挽输出（初始电平低），本文件跟着改成实际端口/引脚即可。*/
#define BOARD_FAN_PORT GPIOC
#define BOARD_FAN_PIN0 GPIO_PIN_6
#define BOARD_FAN_PIN1 GPIO_PIN_7
#define BOARD_FAN_ON_LEVEL 1

/* ---------------- DHT11 单总线 ----------------
 * 引脚名沿用 CubeMX 生成的 main.h（.ioc: PA15 / GPIO_Output OpenDrain）*/
#define BOARD_DHT11_PORT DHT11_DATA_GPIO_Port
#define BOARD_DHT11_PIN DHT11_DATA_Pin

/* ---------------- 物模型映射 ----------------
 * 「灯 1/2/3」→ led.c 的 LED0/LED1/LED2 的对应关系在 APP/Src/app_device.c 的
 * s_actuators 表里（那是业务映射，不是板级引脚信息）。*/

#endif /* __BOARD_PINS_H */
