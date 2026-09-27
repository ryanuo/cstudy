#ifndef __AT24C02_H
#define __AT24C02_H

#include <stdint.h>

/*
 * AT24C02 设备地址
 *
 * 8 位地址：
 *
 * 写：0xA0
 * 读：0xA1
 */
#define AT24C02_ADDR_WRITE   0xA0
#define AT24C02_ADDR_READ    0xA1

/*
 * AT24C02 总容量
 */
#define AT24C02_SIZE         256

/*
 * AT24C02 Page Size
 */
#define AT24C02_PAGE_SIZE    8


void AT24C02_Init(void);

int8_t AT24C02_WriteByte(uint8_t addr, uint8_t data);

int8_t AT24C02_ReadByte(uint8_t addr, uint8_t *data);

int8_t AT24C02_PageWrite(
    uint8_t addr,
    const uint8_t *data,
    uint8_t len
);

int8_t AT24C02_Read(
    uint8_t addr,
    uint8_t *data,
    uint8_t len
);

int8_t AT24C02_WaitReady(void);

#endif