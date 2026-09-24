#include "IIC.h"
#include "DELAY.h"

#define SDA_PIN GPIO_Pin_9
#define SCL_PIN GPIO_Pin_8

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
    }
}