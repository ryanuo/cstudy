#include "beep_web.h"
#include "web.h"
#include "BEEP.h"
#include <string.h>

void Beep_WebState(void)
{
    /* 蜂鸣器无状态可上报 */
}

uint8_t Beep_WebRoute(uint8_t link, const char *path, char *req)
{
    (void)req;

    if (strcmp(path, "beep") == 0)
    {
        BEEP_TriggerNonBlocking(200);
        Web_ReplyOkState(link);
        return 1;
    }

    return 0;
}