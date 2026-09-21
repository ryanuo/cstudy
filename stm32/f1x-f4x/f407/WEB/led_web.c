#include "led_web.h"
#include "web.h"
#include "LED.h"
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

uint8_t Led_WebRoute(uint8_t link, const char *path, char *req)
{
    (void)req;

    if      (strcmp(path, "led1/1") == 0) { LED_FlowEnable(0); LED1_on();  Web_ReplyOkState(link); return 1; }
    else if (strcmp(path, "led1/0") == 0) { LED_FlowEnable(0); LED1_off(); Web_ReplyOkState(link); return 1; }
    else if (strcmp(path, "led2/1") == 0) { LED_FlowEnable(0); LED2_on();  Web_ReplyOkState(link); return 1; }
    else if (strcmp(path, "led2/0") == 0) { LED_FlowEnable(0); LED2_off(); Web_ReplyOkState(link); return 1; }
    else if (strcmp(path, "led3/1") == 0) { LED_FlowEnable(0); LED3_on();  Web_ReplyOkState(link); return 1; }
    else if (strcmp(path, "led3/0") == 0) { LED_FlowEnable(0); LED3_off(); Web_ReplyOkState(link); return 1; }
    else if (strcmp(path, "flow/1") == 0) { LED_FlowEnable(1); Web_ReplyOkState(link); return 1; }
    else if (strcmp(path, "flow/0") == 0) { LED_FlowEnable(0); Web_ReplyOkState(link); return 1; }

    return 0;
}