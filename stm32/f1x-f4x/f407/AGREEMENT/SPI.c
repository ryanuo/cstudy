#include "SPI.h"
#include "DELAY.h"

#define ISHARD 0
#if ISHARD
void SPI1_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
    GPIO_SetBits(GPIOB, GPIO_Pin_14);
}

static uint8_t SPI1_sendbyte(uint8_t byte)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
        ;
    SPI_I2S_SendData(SPI1, byte);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
        ;
    return SPI_I2S_ReceiveData(SPI1);
}
#endif

#if !ISHARD
void SPI1_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_INSTRUCT;
    GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
    GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_14;
    GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_INSTRUCT);
    GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_IN;
    GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOB, &GPIO_INSTRUCT);

    GPIO_SetBits(GPIOB, GPIO_Pin_14);
}

static uint8_t SPI1_sendbyte(uint8_t byte) // 0x78 = 0111 1000
{
    uint8_t result = 0;
    for (int i = 0; i < 8; i++)
    {
        // MOSI发送0或者1
        if (byte & 1 << (7 - i)) // 1
        {
            GPIO_SetBits(GPIOB, GPIO_Pin_5);
        }
        else
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_5);
        }

        GPIO_ResetBits(GPIOB, GPIO_Pin_3);
        DELAY_us(1);
        // MISO接收0或者1
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 1)
        {
            result |= 1 << (7 - i);
        }
        GPIO_SetBits(GPIOB, GPIO_Pin_3);
        DELAY_us(1);
    }
    return result;
}

#endif

void W25QXX_ReadID(uint8_t *id)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_14);
    SPI1_sendbyte(0x9F);
    // SPI1_sendbyte(0x00);
    // SPI1_sendbyte(0x00);
    // SPI1_sendbyte(0x00);
    id[0] = SPI1_sendbyte(0xFF);
    id[1] = SPI1_sendbyte(0xFF);
    id[2] = SPI1_sendbyte(0xFF);
    GPIO_SetBits(GPIOB, GPIO_Pin_14);
}


void W25QXX_Read(uint8_t *buf, uint32_t addr, uint16_t len)
{
    uint16_t i;

    GPIO_ResetBits(GPIOB, GPIO_Pin_14);


    SPI1_sendbyte(0x03);               /* ② 发 0x03 */
    SPI1_sendbyte((uint8_t)(addr >> 16));       /* ③ 地址高 8 位 A23~A16 */
    SPI1_sendbyte((uint8_t)(addr >> 8));        /* ④ 地址中 8 位 A15~A8  */
    SPI1_sendbyte((uint8_t)(addr));             /* ⑤ 地址低 8 位 A7~A0  */

    for (i = 0; i < len; i++) {
        buf[i] = SPI1_sendbyte(0xFF);           /* ⑥ 逐字节读 */
    }

    GPIO_SetBits(GPIOB, GPIO_Pin_14);
}