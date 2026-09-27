#include "led_web.h"
#include "web.h"
#include "LED.h"
#include <string.h>
#include <string.h>

/* 读真实引脚状态（LED 低电平点亮） */
static uint8_t LedOn(GPIO_TypeDef *port, uint16_t pin)
{
    return (GPIO_ReadOutputDataBit(port, pin) == Bit_RESET) ? 1 : 0;
}

void Led_WebState(void)
{
    Web_StateAppend(
        "\"led1\":%u,\"led2\":%u,\"led3\":%u,\"led4\":%u,\"flow\":%u",
        (unsigned)LedOn(GPIOF, GPIO_Pin_9),
        (unsigned)LedOn(GPIOF, GPIO_Pin_10),
        (unsigned)LedOn(GPIOE, GPIO_Pin_13),
        (unsigned)LedOn(GPIOE, GPIO_Pin_14),
        (unsigned)LED_FlowIsEnabled());
}

/* LED 操作函数表 */
typedef void (*LedFunc)(void);

typedef struct {
    const char *path;     /* 路由，如 "led1/1" */
    LedFunc     func;     /* 对应函数 */
    uint8_t     isFlow;   /* 1 = 关流水；0 = 不动流水（flow 路由专用） */
} LedRoute;

static const LedRoute led_routes[] = {
    { "led1/1", LED1_on,  1 },
    { "led1/0", LED1_off, 1 },
    { "led2/1", LED2_on,  1 },
    { "led2/0", LED2_off, 1 },
    { "led3/1", LED3_on,  1 },
    { "led3/0", LED3_off, 1 },
    { "flow/1", NULL,     0 },   /* 特殊：开流水 */
    { "flow/0", NULL,     0 },   /* 特殊：关流水 */
};

#define LED_ROUTE_NUM  (sizeof(led_routes) / sizeof(led_routes[0]))

uint8_t Led_WebRoute(uint8_t link, const char *path, char *req)
{
    uint8_t i;

    (void)req;

    for (i = 0; i < LED_ROUTE_NUM; i++) {
        if (strcmp(path, led_routes[i].path) != 0) continue;

        /* flow/1 和 flow/0 特殊处理 */
        if (strcmp(path, "flow/1") == 0) {
            LED_FlowEnable(1);
        } else if (strcmp(path, "flow/0") == 0) {
            LED_FlowEnable(0);
        } else {
            /* 单灯操作：先关流水，再开/关灯 */
            if (LED_FlowIsEnabled()) LED_FlowEnable(0);
            led_routes[i].func();
        }

        Web_ReplyOkState(link);
        return 1;
    }

    return 0;
}