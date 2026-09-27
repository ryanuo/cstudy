#include "AT24C02.h"
#include "IIC.h"
#include "DELAY.h"

/**
 * @brief 初始化 AT24C02
 */
void AT24C02_Init(void)
{
    IIC_Init();
}

/**
 * @brief 等待 EEPROM 内部写周期完成
 *
 * AT24C02 写入数据后，需要一定时间进行内部写操作。
 *
 * 期间：
 *
 * START
 * 0xA0
 *
 * EEPROM 可能 NACK
 *
 * 写完成之后才会 ACK。
 */
int8_t AT24C02_WaitReady(void)
{
    uint16_t timeout = 0;

    while (timeout < 100)
    {
        IIC_Start();

        IIC_SendByte(AT24C02_ADDR_WRITE);

        if (IIC_WaitAck() == IIC_ACK)
        {
            IIC_Stop();

            return 0;
        }

        IIC_Stop();

        /*
         * 等待一小段时间
         */
        DELAY_us(100);

        timeout++;
    }

    return -1;
}

/**
 * @brief 写一个字节
 *
 * 时序：
 *
 * START
 * A0
 * ACK
 * 地址
 * ACK
 * DATA
 * ACK
 * STOP
 */
int8_t AT24C02_WriteByte(uint8_t addr, uint8_t data)
{
    /*
     * 地址范围检查
     */
    if (addr >= AT24C02_SIZE)
    {
        return -1;
    }

    IIC_Start();

    /*
     * Device Address + Write
     */
    IIC_SendByte(AT24C02_ADDR_WRITE);

    if (IIC_WaitAck() == IIC_NACK)
    {
        IIC_Stop();
        return -2;
    }

    /*
     * Memory Address
     */
    IIC_SendByte(addr);

    if (IIC_WaitAck() == IIC_NACK)
    {
        IIC_Stop();
        return -3;
    }

    /*
     * Data
     */
    IIC_SendByte(data);

    if (IIC_WaitAck() == IIC_NACK)
    {
        IIC_Stop();
        return -4;
    }

    /*
     * STOP
     */
    IIC_Stop();

    /*
     * 等待 EEPROM 内部写完成
     */
    if (AT24C02_WaitReady() != 0)
    {
        return -5;
    }

    return 0;
}

/**
 * @brief 随机读取一个字节
 */
int8_t AT24C02_ReadByte(uint8_t addr, uint8_t *data)
{
    if (data == 0)
    {
        return -1;
    }

    if (addr >= AT24C02_SIZE)
    {
        return -2;
    }

    /*
     * 第一步：
     *
     * 告诉 EEPROM 我要读取哪个地址
     */

    IIC_Start();

    /*
     * 写地址
     */
    IIC_SendByte(AT24C02_ADDR_WRITE);

    if (IIC_WaitAck() == IIC_NACK)
    {
        IIC_Stop();
        return -3;
    }

    /*
     * Memory Address
     */
    IIC_SendByte(addr);

    if (IIC_WaitAck() == IIC_NACK)
    {
        IIC_Stop();
        return -4;
    }

    /*
     * 第二步：
     *
     * Restart
     */
    IIC_Start();

    /*
     * 读地址
     */
    IIC_SendByte(AT24C02_ADDR_READ);

    if (IIC_WaitAck() == IIC_NACK)
    {
        IIC_Stop();
        return -5;
    }

    /*
     * 读取数据
     */
    *data = IIC_ReceiveByte();

    /*
     * 最后一个字节必须 NACK
     */
    IIC_SendAck(IIC_NACK);

    /*
     * STOP
     */
    IIC_Stop();

    return 0;
}

/**
 * @brief Page Write
 *
 * AT24C02 每页 8 字节。
 *
 * 注意：
 *
 * 不能跨页写。
 *
 * 例如：
 *
 * addr = 0x06
 * len  = 4
 *
 * 0x06
 * 0x07
 * 0x08
 * 0x09
 *
 * 这是跨页写。
 *
 * 本函数直接拒绝。
 */
int8_t AT24C02_PageWrite(
    uint8_t addr,
    const uint8_t *data,
    uint8_t len)
{
    uint8_t page_offset;

    if (data == 0)
    {
        return -1;
    }

    if (len == 0)
    {
        return -2;
    }

    if (addr >= AT24C02_SIZE)
    {
        return -3;
    }

    if ((uint16_t)addr + len > AT24C02_SIZE)
    {
        return -4;
    }

    /*
     * 当前地址在页中的偏移
     */
    page_offset = addr % AT24C02_PAGE_SIZE;

    /*
     * 不能跨页
     */
    if ((uint16_t)page_offset + len > AT24C02_PAGE_SIZE)
    {
        return -5;
    }

    /*
     * START
     */
    IIC_Start();

    /*
     * Device Address + Write
     */
    IIC_SendByte(AT24C02_ADDR_WRITE);

    if (IIC_WaitAck() == IIC_NACK)
    {
        IIC_Stop();
        return -6;
    }

    /*
     * Memory Address
     */
    IIC_SendByte(addr);

    if (IIC_WaitAck() == IIC_NACK)
    {
        IIC_Stop();
        return -7;
    }

    /*
     * 连续写数据
     */
    while (len--)
    {
        IIC_SendByte(*data++);

        if (IIC_WaitAck() == IIC_NACK)
        {
            IIC_Stop();
            return -8;
        }
    }

    /*
     * STOP
     */
    IIC_Stop();

    /*
     * 等待内部写周期完成
     */
    if (AT24C02_WaitReady() != 0)
    {
        return -9;
    }

    return 0;
}

/**
 * @brief 连续读取多个字节
 *
 * 时序：
 *
 * START
 * A0
 * ACK
 * Address
 * ACK
 * RESTART
 * A1
 * ACK
 *
 * DATA
 * ACK
 *
 * DATA
 * ACK
 *
 * DATA
 * NACK
 *
 * STOP
 */
int8_t AT24C02_Read(
    uint8_t addr,
    uint8_t *data,
    uint8_t len)
{
    uint8_t i;

    if (data == 0)
    {
        return -1;
    }

    if (len == 0)
    {
        return -2;
    }

    if (addr >= AT24C02_SIZE)
    {
        return -3;
    }

    if ((uint16_t)addr + len > AT24C02_SIZE)
    {
        return -4;
    }

    /*
     * START
     */
    IIC_Start();

    /*
     * 写地址
     */
    IIC_SendByte(AT24C02_ADDR_WRITE);

    if (IIC_WaitAck() == IIC_NACK)
    {
        IIC_Stop();
        return -5;
    }

    /*
     * Memory Address
     */
    IIC_SendByte(addr);

    if (IIC_WaitAck() == IIC_NACK)
    {
        IIC_Stop();
        return -6;
    }

    /*
     * RESTART
     */
    IIC_Start();

    /*
     * 读地址
     */
    IIC_SendByte(AT24C02_ADDR_READ);

    if (IIC_WaitAck() == IIC_NACK)
    {
        IIC_Stop();
        return -7;
    }

    /*
     * 连续读取
     */
    for (i = 0; i < len; i++)
    {
        data[i] = IIC_ReceiveByte();

        /*
         * 最后一个字节发送 NACK
         *
         * 前面的字节发送 ACK
         */
        if (i < len - 1)
        {
            IIC_SendAck(IIC_ACK);
        }
        else
        {
            IIC_SendAck(IIC_NACK);
        }
    }

    /*
     * STOP
     */
    IIC_Stop();

    return 0;
}