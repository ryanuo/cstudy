#ifndef __WEB_H
#define __WEB_H

#include "stm32f4xx.h"

/* 令牌：和原来一样，改这里即可 */
#define WEB_TOKEN  "stm32-407"

/* ================= 基础设施 API（业务模块只用这些） ================= */

/* 请求解析 */
uint8_t  Web_TokenOk(char *req);
uint8_t  Web_PathOf(char *req, char *path, uint8_t max);
uint32_t Web_GetParamU32(char *req, const char *key, uint32_t def);

/* 回复 */
void     Web_ReplyJson(uint8_t link, const char *body, uint16_t blen);
void     Web_ReplyOk(uint8_t link);              /* {"ok":1} */
void     Web_ReplyOkState(uint8_t link);         /* {"ok":1, + 全状态} */
void     Web_ReplyErr(uint8_t link);             /* {"err":1} */
void     Web_CloseLink(uint8_t link);

/* /data 聚合 */
void     Web_StateReset(void);
void     Web_StateAppend(const char *fmt, ...);
void     Web_StateSend(uint8_t link);
void     Web_SendAllState(uint8_t link);

/* 生命周期 */
uint8_t  Web_Init(void);
uint8_t  Web_Connect(char *ssid, char *pass);
uint8_t  Web_GetIp(char *ip, uint8_t max_len);
uint8_t  Web_OpenServer(uint16_t port);
/* 自愈：清掉残链接并重开服务器（静默一段时间后调用一次） */
uint8_t Web_ResetServer(uint16_t port);
void     Web_Task(void);
uint16_t Web_ReqCount(void);

/* ================= 业务模块注册（各模块实现） ================= */

void    Led_WebState(void);
uint8_t Led_WebRoute(uint8_t link, const char *path, char *req);

void    Fan_WebState(void);
uint8_t Fan_WebRoute(uint8_t link, const char *path, char *req);

void    Sensor_WebState(void);
uint8_t Sensor_WebRoute(uint8_t link, const char *path, char *req);

void    Beep_WebState(void);
uint8_t Beep_WebRoute(uint8_t link, const char *path, char *req);

uint8_t Flash_WebRoute(uint8_t link, const char *path, char *req);

#endif