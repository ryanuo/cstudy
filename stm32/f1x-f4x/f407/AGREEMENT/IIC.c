#include "IIC.h"
#include "DELAY.h"

#define SDA_PIN GPIO_Pin_9
#define SCL_PIN GPIO_Pin_8
#define IIC_ACK 0
#define IIC_NACK 1

void IIC_setsdamode(GPIOMode_TypeDef mode)
{
    // Implementation for setting SDA mode
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_Pin = SDA_PIN; // Assuming SDA is on Pin 9
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void IIC_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    IIC_setsdamode(GPIO_Mode_OUT);

    GPIO_SetBits(GPIOB, SCL_PIN);
    GPIO_SetBits(GPIOB, SDA_PIN);
}

void IIC_start(void)
{
    IIC_setsdamode(GPIO_Mode_OUT);
    GPIO_SetBits(GPIOB, SCL_PIN);
    GPIO_SetBits(GPIOB, SDA_PIN);

    GPIO_ResetBits(GPIOB, SDA_PIN);
    DELAY_us(4);
    GPIO_ResetBits(GPIOB, SCL_PIN);
    DELAY_us(4);
}

void IIC_stop(void)
{
    IIC_setsdamode(GPIO_Mode_OUT);
    GPIO_ResetBits(GPIOB, SCL_PIN);
    GPIO_ResetBits(GPIOB, SDA_PIN);

    GPIO_SetBits(GPIOB, SCL_PIN);
    DELAY_us(4);
    GPIO_SetBits(GPIOB, SDA_PIN);
    DELAY_us(4);
}

// 主机向从机发送一个字节数据
void IIC_sendbyte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (byte & (1 << 7 - i))
        {
            GPIO_SetBits(GPIOB, SDA_PIN);
        }
        else
        {
            GPIO_ResetBits(GPIOB, SDA_PIN);
        }

        DELAY_us(4);
        GPIO_SetBits(GPIOB, SCL_PIN);
        DELAY_us(4);
        GPIO_ResetBits(GPIOB, SCL_PIN);
        DELAY_us(4);
    }
}

// 主机等待从机应答信号
uint8_t IIC_waitack(void)
{
    IIC_setsdamode(GPIO_Mode_IN);
    GPIO_SetBits(GPIOB, SCL_PIN);
    DELAY_us(4);

    if (GPIO_ReadInputDataBit(GPIOB, SDA_PIN) == SET)
    {
        GPIO_ResetBits(GPIOB, SCL_PIN);
        DELAY_us(4);
        return IIC_NACK; // No ACK received
    }

    GPIO_ResetBits(GPIOB, SCL_PIN);
    DELAY_us(4);
    return IIC_ACK; // ACK received
}

// 主机从从机接收一个字节数据
uint8_t IIC_receivebyte(void)
{
    uint8_t data = 0;
    IIC_setsdamode(GPIO_Mode_IN);
    for (uint8_t i = 0; i < 8; i++)
    {
        GPIO_SetBits(GPIOB, SCL_PIN);
        DELAY_us(4);
        if (GPIO_ReadInputDataBit(GPIOB, SDA_PIN) == SET)
        {
            data |= 1 << (7 - i);
        }
        GPIO_ResetBits(GPIOB, SCL_PIN);
        DELAY_us(4);
    }
    return data;
}

// 主机向从机发送应答信号
void IIC_sendack(uint8_t ack)
{
    IIC_setsdamode(GPIO_Mode_OUT);
    GPIO_ResetBits(GPIOB, SDA_PIN);
    GPIO_ResetBits(GPIOB, SCL_PIN);

    if (ack)
    {
        GPIO_ResetBits(GPIOB, SDA_PIN); // Send ACK
    }
    else
    {
        GPIO_SetBits(GPIOB, SDA_PIN); // Send NACK
    }

    DELAY_us(4);
    GPIO_SetBits(GPIOB, SCL_PIN);
    DELAY_us(4);
    GPIO_ResetBits(GPIOB, SCL_PIN);
    DELAY_us(4);
}

int8_t AT2402_pagewrite(uint8_t slave, uint16_t addr, uint8_t *data, uint8_t len)
{
    IIC_start();
    IIC_sendbyte(slave);
    if (IIC_waitack() == IIC_NACK)
    {
        IIC_stop();
        return -1; // No ACK received
    }
    // Continue with the rest of the implementation...
    IIC_sendbyte(slave);
    if (IIC_waitack() == IIC_NACK)
    {
        IIC_stop();
        return -2; // No ACK received
    }

    while (len--)
    {
        IIC_sendbyte(*data++);
        if (IIC_waitack() == IIC_NACK)
        {
            IIC_stop();
            return -3; // No ACK received
        }
    }
    IIC_stop();
    return 0; // Success
}

int8_t IIC_randomread(uint8_t slave, uint8_t address, uint8_t *data, uint8_t len)
{
    IIC_start();
    IIC_sendbyte(slave); // 0xA0 1010 000 0

    if (IIC_waitack() == IIC_NACK)
    {
        IIC_stop();
        return -4; // No ACK received
    }

    IIC_sendbyte(address);
    if (IIC_waitack() == IIC_NACK)
    {
        IIC_stop();
        return -5; // No ACK received
    }

    IIC_start();
    IIC_sendbyte(slave | 0x01);
    if (IIC_waitack() == IIC_NACK)
    {
        IIC_stop();
        return -6; // No ACK received
    }

    while (len--)
    {
        *data++ = IIC_receivebyte();
        if (len)
        {
            IIC_sendack(IIC_ACK);
        }
        else
        {
            IIC_sendack(IIC_NACK);
        }
    }
    IIC_stop();
    return 0; // Success
}