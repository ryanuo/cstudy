#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

/* ============================================================
 *  应用级配置 —— 唯一真值源
 *  凭据、周期、调试开关都放这里，别散落到 main.c / app_*.c
 *  （引脚相关配置在 BSP/Inc/board_pins.h）
 * ============================================================ */

/* ---------------- WiFi ---------------- */
#define APP_WIFI_SSID "YQ-shixun5"
#define APP_WIFI_PWD "88888888"

/* ---------------- OneNET 设备 ---------------- */
#define APP_ONENET_PRODUCT_ID "WW0f6843I6"
#define APP_ONENET_DEVICE_ID "humi_temp"

/* 属性上报用 Token（OneNET Studio 生成，注意 et 有效期） */
#define APP_ONENET_TOKEN                                                       \
  "version=2018-10-31&res=products%2FWW0f6843I6%2Fdevices%2Fhumi_temp&"        \
  "et=1822562760&method=sha1&sign=o3KQXG0XbEBbJZBHkyfXDpzSIHQ%3D"

/* ---------------- 运行周期 ---------------- */
#define APP_REPORT_PERIOD_MS 5000U    /* 属性上报周期 */
#define APP_DHT11_PERIOD_MS 2000U     /* DHT11 采样周期（bsp_dht11.c 内部节流用） */

/* ---------------- 物模型 ----------------
 * 与 OneNET 控制台里的定义必须逐字一致（否则平台回 10411 identifier not exist）：
 *   led1 / led2 / led3   string(8)  取值 "on" / "off"（★ 小写，不是 LED1）
 *   buzzer / fan         bool       true / false
 *   temperature / humidity  number   DHT11 读数
 *   服务 reboot          ——         面板按钮触发，设备先回应答再复位
 * 详见工程根 README.md 的「物模型」一节。*/
/* 1 = 上报全部执行器（灯 1/2/3 + 蜂鸣器 + 风扇）
 * 物模型里这五个属性都已存在（led1/led2/led3 是 string，buzzer/fan 是 bool），
 * 所以可以全量上报；上电连上 OneNET 后与每 5 秒的周期上报都会带上它们的真实状态。*/
#define APP_REPORT_ACTUATOR_STATE 1

/* ---------------- 调试开关 ---------------- */
#define APP_ONENET_DEBUG 0 /* 1 = 每条 AT 后把 ESP 原始缓冲 printf 出来（同步阻塞，很慢）*/

#endif /* __APP_CONFIG_H */
