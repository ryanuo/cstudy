#include "W25QXX.h"

void W25Q_CS_LOW(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_14);
}
void W25Q_CS_HIGH(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_14);
}

void W25QXX_ReadID(uint8_t *id)
{
    W25Q_CS_LOW();
    SPI1_sendbyte(0x9F);
    // SPI1_sendbyte(0x00);
    // SPI1_sendbyte(0x00);
    // SPI1_sendbyte(0x00);
    id[0] = SPI1_sendbyte(0xFF);
    id[1] = SPI1_sendbyte(0xFF);
    id[2] = SPI1_sendbyte(0xFF);
    W25Q_CS_HIGH();
}

void W25QXX_Read(uint8_t *buf, uint32_t addr, uint16_t len)
{
    uint16_t i;

    W25Q_CS_LOW();

    SPI1_sendbyte(0x03);                  /* ② 发 0x03 */
    SPI1_sendbyte((uint8_t)(addr >> 16)); /* ③ 地址高 8 位 A23~A16 */
    SPI1_sendbyte((uint8_t)(addr >> 8));  /* ④ 地址中 8 位 A15~A8  */
    SPI1_sendbyte((uint8_t)(addr));       /* ⑤ 地址低 8 位 A7~A0  */

    for (i = 0; i < len; i++)
    {
        buf[i] = SPI1_sendbyte(0xFF); /* ⑥ 逐字节读 */
    }

    W25Q_CS_HIGH();
}

uint8_t W25QXX_ReadSR(void)
{
    uint8_t sr;

    W25Q_CS_LOW();
    /* ① CS 拉低 */
    SPI1_sendbyte(0x05);              /* ② 发 0x05 */
    sr = SPI1_sendbyte(0xFF);         /* ③ 读 1 字节 */
    W25Q_CS_HIGH(); /* ④ CS 拉高 */

    return sr;
}

/* ============================================================
 * 等 BUSY 清零
 * ============================================================ */
void W25QXX_WaitBusy(void)
{
    while (W25QXX_ReadSR() & 0x01)
    {
    }
}

/* ============================================================
 * 写使能（0x06）—— 每次擦除/编程前必须发
 * ============================================================ */
void W25QXX_WriteEnable(void)
{
    W25Q_CS_LOW();
    SPI1_sendbyte(0x06); /* 0x06 */
    W25Q_CS_HIGH();
}

void W25QXX_Erase_Sector(uint32_t addr)
{
    W25QXX_WriteEnable(); /* ① 发 0x06，WEL=1 */

    W25Q_CS_LOW(); /* ② CS 拉低 */

    SPI1_sendbyte(0x20);                  /* ③ 发 0x20 */
    SPI1_sendbyte((uint8_t)(addr >> 16)); /* ④ 地址高字节 */
    SPI1_sendbyte((uint8_t)(addr >> 8));  /* ⑤ 地址中字节 */
    SPI1_sendbyte((uint8_t)(addr));       /* ⑥ 地址低字节 */

    W25Q_CS_HIGH(); /* ⑦ CS 拉高，触发擦除 */

    W25QXX_WaitBusy(); /* ⑧ 轮询 BUSY */
}


/* ============================================================
 * 页编程（0x02）—— 往 addr 写 len 字节
 * 限制：一页 256 字节，不能跨页；写前区域必须已擦除
 * ============================================================ */
void W25QXX_PageProgram(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint16_t i;

    W25QXX_WriteEnable();          /* ① 写使能 0x06 */

    W25Q_CS_LOW();                 /* ② CS 拉低 */

    SPI1_sendbyte(0x02);           /* ③ 页编程指令 */
    SPI1_sendbyte((uint8_t)(addr >> 16));  /* ④ 地址 A23~A16 */
    SPI1_sendbyte((uint8_t)(addr >> 8));   /* ⑤ 地址 A15~A8  */
    SPI1_sendbyte((uint8_t)(addr));        /* ⑥ 地址 A7~A0   */

    for (i = 0; i < len; i++)      /* ⑦ 发数据 */
    {
        SPI1_sendbyte(buf[i]);
    }

    W25Q_CS_HIGH();                /* ⑧ CS 拉高，触发写入 */

    W25QXX_WaitBusy();             /* ⑨ 等内部写完 */
}

void W25QXX_Write(uint32_t addr, uint8_t *buf, uint16_t len)
{
    W25QXX_Erase_Sector(addr);          /* 先擦整个扇区 */
    W25QXX_PageProgram(addr, buf, len); /* 再写 */
}