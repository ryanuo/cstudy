#include "fan_web.h"
#include "web.h"
#include "FAN.h"
#include <string.h>

static uint8_t FanState(void)
{
    uint8_t p = FAN_ReadPins();

    if (p == 0x02) return 1;   /* 正转 */
    if (p == 0x01) return 2;   /* 反转 */
    return 0;
}

void Fan_WebState(void)
{
    Web_StateAppend("\"fan\":%u", (unsigned)FanState());
}

uint8_t Fan_WebRoute(uint8_t link, const char *path, char *req)
{
    (void)req;

    if      (strcmp(path, "fan/1") == 0) { FAN_forwardrotation(); Web_ReplyOkState(link); return 1; }
    else if (strcmp(path, "fan/2") == 0) { FAN_reverserotation(); Web_ReplyOkState(link); return 1; }
    else if (strcmp(path, "fan/0") == 0) { FAN_off();             Web_ReplyOkState(link); return 1; }

    return 0;
}