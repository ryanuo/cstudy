#include "stm32f4xx.h"                  // Device header
#include "SPI.h"
#include "BITBAND.h"
#include "DELAY.h"
//////////////////SPI底层代码////////////////////
#if 0
void SPI1HARDWARE_init(void)
{
//1）使能GPIOB的时钟
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
//2）配置模式(输出模式1，复用模式3)
GPIO_InitTypeDef GPIO_INSTRUCT;
GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;
GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_14;
GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
GPIO_INSTRUCT.GPIO_Speed =GPIO_Speed_100MHz;
GPIO_Init(GPIOB,&GPIO_INSTRUCT);
GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_AF;
GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 |GPIO_Pin_5;
//3）使GPIO配置生效
GPIO_Init(GPIOB,&GPIO_INSTRUCT);
//4）使能SPI的时钟
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
//5）将GPIO复用为SPI1
GPIO_PinAFConfig(GPIOB,GPIO_PinSource3,GPIO_AF_SPI1);
GPIO_PinAFConfig(GPIOB,GPIO_PinSource4,GPIO_AF_SPI1);
GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_SPI1);
//6）配置SPI的结构体
SPI_InitTypeDef SPI_INSTRUCT;
SPI_INSTRUCT.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//84MH/8=10.5M
SPI_INSTRUCT.SPI_CPHA = SPI_CPHA_1Edge;//时钟相位
SPI_INSTRUCT.SPI_CPOL = SPI_CPOL_Low;//时钟极性
SPI_INSTRUCT.SPI_CRCPolynomial = 7;//CRC校验
SPI_INSTRUCT.SPI_DataSize = SPI_DataSize_8b;//8bit
SPI_INSTRUCT.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//全双工
SPI_INSTRUCT.SPI_FirstBit = SPI_FirstBit_MSB;//高位先出
SPI_INSTRUCT.SPI_Mode = SPI_Mode_Master;//主机模式
SPI_INSTRUCT.SPI_NSS = SPI_NSS_Soft;//软件片选
//7）使SPI结构体配置生效
SPI_Init(SPI1,&SPI_INSTRUCT);
//8）启动SPI
SPI_Cmd(SPI1,ENABLE);
PBout(14) = 1;//避免立即触发
}

//9）SPI核心通讯的底层函数----数据置换
static uint8_t SPI1_sendbyte(uint8_t byte)
{
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1,byte);
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI1);
}
#endif
void SPI1SOFTWARE_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_14;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB,&GPIO_INSTRUCT);
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_IN;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOB,&GPIO_INSTRUCT);
	PBout(14) = 1;//避免立即触发
}

uint8_t SPI1_sendbyte(uint8_t byte)//0x78 = 0111 1000
{
	uint8_t result = 0;
  for(int i = 0;i<8;i++)
	{
		//MOSI发送0或者1
		if(byte&1<<(7-i))//1
		{
			PBout(5) = 1;
		}
    else
		{
			PBout(5) = 0;
		}
     
		PBout(3) = 0;
		DELAY_us(2);
		//MISO接收0或者1
		if(PBin(4) == 1)
		{
		   result |= 1<<(7-i);
		}
		PBout(3) = 1;
		DELAY_us(2);
	}
	return result;
}
