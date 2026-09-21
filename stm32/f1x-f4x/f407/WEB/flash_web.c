#include "flash_web.h"
#include "web.h"
#include "SPI.h" /* W25QXX 驱动 */
#include <string.h>
#include <stdio.h>

/* ---------------- 芯片识别 ---------------- */

static const char *FlashName(uint8_t mfr, uint8_t mem)
{
    if (mfr == 0xEF)
    { /* Winbond */
        switch (mem)
        {
        case 0x15:
            return "W25Q16";
        case 0x16:
            return "W25Q32";
        case 0x17:
            return "W25Q64";
        case 0x18:
            return "W25Q128";
        case 0x40:
            return "W25Q128";
        case 0x60:
            return "W25Q256";
        case 0x70:
            return "W25Q512";
        }
    }
    if (mfr == 0xC8)
    { /* GigaDevice */
        switch (mem)
        {
        case 0x16:
            return "GD25Q32";
        case 0x17:
            return "GD25Q64";
        case 0x18:
            return "GD25Q128";
        case 0x40:
            return "GD25Q128";
        }
    }
    if (mfr == 0x1C)
    { /* EON */
        switch (mem)
        {
        case 0x18:
            return "EN25Q128";
        case 0x40:
            return "EN25Q128";
        }
    }
    return "Unknown";
}

static uint32_t FlashSize(uint8_t mem)
{
    switch (mem)
    {
    case 0x14:
        return 1 * 1024 * 1024;
    case 0x15:
        return 2 * 1024 * 1024;
    case 0x16:
        return 4 * 1024 * 1024;
    case 0x17:
        return 8 * 1024 * 1024;
    case 0x18:
        return 16 * 1024 * 1024;
    case 0x40:
        return 16 * 1024 * 1024;
    case 0x60:
        return 32 * 1024 * 1024;
    case 0x70:
        return 64 * 1024 * 1024;
    }
    return 0;
}

/* ---------------- 路由 ---------------- */

uint8_t Flash_WebRoute(uint8_t link, const char *path, char *req)
{
    /* /flash/id */
    if (strcmp(path, "flash/id") == 0)
    {
        static char json[128];
        uint8_t id[3];

        W25QXX_ReadID(id);
        sprintf(json, "{\"id\":\"%02X%02X%02X\",\"name\":\"%s\",\"size\":%u}",
                id[0], id[1], id[2],
                FlashName(id[0], id[1]),
                (unsigned)FlashSize(id[2]));
        Web_ReplyJson(link, json, (uint16_t)strlen(json));
        return 1;
    }

    /* /flash/read?addr=0&len=128 */
    if (strcmp(path, "flash/read") == 0)
    {
        static char json[768];
        static uint8_t buf[256];
        uint32_t addr = Web_GetParamU32(req, "addr", 0);
        uint32_t len = Web_GetParamU32(req, "len", 128);
        char *p;
        uint16_t i;

        if (len > 256)
            len = 256;
        /* 外挂 W25Q128 的地址空间是 0x000000~0xFFFFFF（用"指令+偏移"访问，不是内存映射）。
           以前超范围是静默夹成 0xFFFFFF：用户填 0x08000000（STM32 内部 flash 的基址）时，
           界面上的基址会莫名其妙变成 00FFFFFF，看不出是被改了。现在直接回明确原因。 */
        if (addr > 0xFFFFFF)
        {
            sprintf(json, "{\"err\":1,\"why\":\"addr 超出外挂 W25Q128 的 0x000000-0xFFFFFF；0x08000000 是 STM32 内部 flash（点读取会自动走那条）\",\"max\":%u}",
                    (unsigned)0xFFFFFF);
            Web_ReplyJson(link, json, (uint16_t)strlen(json));
            return 1;
        }

        W25QXX_Read(buf, addr, (uint16_t)len);

        p = json;
        p += sprintf(p, "{\"addr\":%u,\"len\":%u,\"hex\":\"",
                     (unsigned)addr, (unsigned)len);
        for (i = 0; i < len; i++)
            p += sprintf(p, "%02X", buf[i]);
        sprintf(p, "\"}");

        Web_ReplyJson(link, json, (uint16_t)strlen(json));
        return 1;
    }

    /* /flash/mcu?addr=0x08000000&len=128
       —— 读 STM32 内部 flash：内存直读（外挂 SPI flash 没有地址映射，读不了 0x08000000）*/
    if (strcmp(path, "flash/mcu") == 0)
    {
        static char    json[768];
        static uint8_t buf[256];
        uint32_t       addr = Web_GetParamU32(req, "addr", 0x08000000u);
        uint32_t       len  = Web_GetParamU32(req, "len", 128);
        char          *p;
        uint16_t       i;

        if (len > 256)
            len = 256;
        if (addr < 0x08000000u || addr > 0x0807FFFFu)          /* F407ZE 内部 flash 512KB */
            addr = 0x08000000u;
        if (addr + len > 0x08080000u)
            len = 0x08080000u - addr;

        for (i = 0; i < len; i++)
            buf[i] = *(volatile uint8_t *)(addr + i);

        p = json;
        p += sprintf(p, "{\"addr\":%u,\"len\":%u,\"hex\":\"", (unsigned)addr, (unsigned)len);
        for (i = 0; i < len; i++)
            p += sprintf(p, "%02X", buf[i]);
        sprintf(p, "\"}");

        Web_ReplyJson(link, json, (uint16_t)strlen(json));
        return 1;
    }

    /* /flash/erase?addr=0 */
    if (strcmp(path, "flash/erase") == 0)
    {
        uint32_t addr = Web_GetParamU32(req, "addr", 0) & ~0xFFFu;

        W25QXX_Erase_Sector(addr);
        Web_ReplyOk(link);
        return 1;
    }

    /* /flash/status */
    if (strcmp(path, "flash/status") == 0)
    {
        static char json[64];
        uint8_t     st = W25QXX_ReadSR();

        sprintf(json, "{\"busy\":%u,\"wel\":%u}",
                (unsigned)(st & 0x01),
                (unsigned)((st >> 1) & 0x01));
        Web_ReplyJson(link, json, (uint16_t)strlen(json));
        return 1;
    }

    return 0;
}