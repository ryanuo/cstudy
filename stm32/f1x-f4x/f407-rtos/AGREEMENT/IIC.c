#include "IIC.h"
#include "DELAY.h"

/*
 * STM32F407
 *
 * PB8 -> SCL
 * PB9 -> SDA
 */

#define IIC_PORT        GPIOB
#define IIC_SCL_PIN     GPIO_Pin_8
#define IIC_SDA_PIN     GPIO_Pin_9


/**
 * @brief 设置 SDA 输入/输出模式
 */
static void IIC_SetSdaMode(GPIOMode_TypeDef mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin   = IIC_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode  = mode;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(IIC_PORT, &GPIO_InitStructure);
}


/**
 * @brief 初始化 I2C GPIO
 */
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 开启 GPIOB 时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    /*
     * SCL
     */
    GPIO_InitStructure.GPIO_Pin   = IIC_SCL_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(IIC_PORT, &GPIO_InitStructure);

    /*
     * SDA
     */
    IIC_SetSdaMode(GPIO_Mode_OUT);

    /*
     * 总线空闲状态：
     *
     * SCL = 1
     * SDA = 1
     */
    GPIO_SetBits(IIC_PORT, IIC_SCL_PIN);
    GPIO_SetBits(IIC_PORT, IIC_SDA_PIN);
}


/**
 * @brief I2C 起始信号
 *
 * SDA: 1 -> 0
 * SCL: 1
 */
void IIC_Start(void)
{
    IIC_SetSdaMode(GPIO_Mode_OUT);

    GPIO_SetBits(IIC_PORT, IIC_SCL_PIN);
    GPIO_SetBits(IIC_PORT, IIC_SDA_PIN);

    DELAY_us(2);

    GPIO_ResetBits(IIC_PORT, IIC_SDA_PIN);

    DELAY_us(2);

    GPIO_ResetBits(IIC_PORT, IIC_SCL_PIN);

    DELAY_us(2);
}


/**
 * @brief I2C 停止信号
 *
 * SDA: 0 -> 1
 * SCL: 1
 */
void IIC_Stop(void)
{
    IIC_SetSdaMode(GPIO_Mode_OUT);

    GPIO_ResetBits(IIC_PORT, IIC_SCL_PIN);
    GPIO_ResetBits(IIC_PORT, IIC_SDA_PIN);

    DELAY_us(2);

    GPIO_SetBits(IIC_PORT, IIC_SCL_PIN);

    DELAY_us(2);

    GPIO_SetBits(IIC_PORT, IIC_SDA_PIN);

    DELAY_us(2);
}


/**
 * @brief 发送一个字节
 */
void IIC_SendByte(uint8_t byte)
{
    uint8_t i;

    IIC_SetSdaMode(GPIO_Mode_OUT);

    for (i = 0; i < 8; i++)
    {
        /*
         * 从最高位开始发送
         */
        if (byte & (1U << (7 - i)))
        {
            GPIO_SetBits(IIC_PORT, IIC_SDA_PIN);
        }
        else
        {
            GPIO_ResetBits(IIC_PORT, IIC_SDA_PIN);
        }

        DELAY_us(2);

        /*
         * SCL 拉高
         */
        GPIO_SetBits(IIC_PORT, IIC_SCL_PIN);

        DELAY_us(4);

        /*
         * SCL 拉低
         */
        GPIO_ResetBits(IIC_PORT, IIC_SCL_PIN);

        DELAY_us(2);
    }
}


/**
 * @brief 等待从机 ACK
 *
 * ACK  = SDA = 0
 * NACK = SDA = 1
 *
 * @return IIC_ACK / IIC_NACK
 */
uint8_t IIC_WaitAck(void)
{
    uint8_t ack;

    /*
     * 释放 SDA
     */
    IIC_SetSdaMode(GPIO_Mode_IN);

    /*
     * SCL 拉高
     */
    GPIO_SetBits(IIC_PORT, IIC_SCL_PIN);

    DELAY_us(4);

    /*
     * 读取 SDA
     */
    if (GPIO_ReadInputDataBit(IIC_PORT, IIC_SDA_PIN) == RESET)
    {
        ack = IIC_ACK;
    }
    else
    {
        ack = IIC_NACK;
    }

    /*
     * SCL 拉低
     */
    GPIO_ResetBits(IIC_PORT, IIC_SCL_PIN);

    DELAY_us(2);

    return ack;
}


/**
 * @brief 接收一个字节
 */
uint8_t IIC_ReceiveByte(void)
{
    uint8_t i;
    uint8_t data = 0;

    /*
     * 释放 SDA，让从机控制 SDA
     */
    IIC_SetSdaMode(GPIO_Mode_IN);

    for (i = 0; i < 8; i++)
    {
        /*
         * SCL 拉高
         */
        GPIO_SetBits(IIC_PORT, IIC_SCL_PIN);

        DELAY_us(4);

        /*
         * 读取 SDA
         */
        if (GPIO_ReadInputDataBit(IIC_PORT, IIC_SDA_PIN) == SET)
        {
            data |= (1U << (7 - i));
        }

        /*
         * SCL 拉低
         */
        GPIO_ResetBits(IIC_PORT, IIC_SCL_PIN);

        DELAY_us(2);
    }

    return data;
}


/**
 * @brief 主机发送 ACK / NACK
 */
void IIC_SendAck(uint8_t ack)
{
    IIC_SetSdaMode(GPIO_Mode_OUT);

    /*
     * ACK  -> SDA = 0
     * NACK -> SDA = 1
     */
    if (ack == IIC_ACK)
    {
        GPIO_ResetBits(IIC_PORT, IIC_SDA_PIN);
    }
    else
    {
        GPIO_SetBits(IIC_PORT, IIC_SDA_PIN);
    }

    DELAY_us(2);

    /*
     * SCL 高
     */
    GPIO_SetBits(IIC_PORT, IIC_SCL_PIN);

    DELAY_us(4);

    /*
     * SCL 低
     */
    GPIO_ResetBits(IIC_PORT, IIC_SCL_PIN);

    DELAY_us(2);
}