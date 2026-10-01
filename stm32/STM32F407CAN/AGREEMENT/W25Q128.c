#include "stm32f4xx.h"                  // Device header
#include "SPI.h"
#include "W25Q128.h"
#include "BITBAND.h"
#include "DELAY.h"
uint16_t W25Q128_getmanddevice(void)
{
	PBout(14) = 0;
	SPI1_sendbyte(0X90);//90h是代表16进制数据
	SPI1_sendbyte(0X00);//0x000000
	SPI1_sendbyte(0X00);
	SPI1_sendbyte(0X01);
	uint8_t mand =  SPI1_sendbyte(0X7C);
	uint8_t device =  SPI1_sendbyte(0X7d);
	PBout(14) = 1;
	return (mand<<8 |device );
}

void W25Q128_writeenable(void)
{
	PBout(14) = 0;
	SPI1_sendbyte(0X06);
	PBout(14) = 1;
}

void W25Q128_writedisable(void)
{
	PBout(14) = 0;
	SPI1_sendbyte(0X04);
	PBout(14) = 1;
}
uint8_t W25Q128_readstatus1(void)
{
	uint8_t result = 0;
	PBout(14) = 0;
	SPI1_sendbyte(0X05);
	result = SPI1_sendbyte(0X17);
	PBout(14) = 1;
	return result;
}
void W25Q128_erasesector(uint32_t address)
{
	uint32_t sector = address/4096;//扇区编号
	uint32_t sectoraddress = sector * 4096;//扇区对应的起始地址
	W25Q128_writeenable();
	PBout(14) = 0;
	SPI1_sendbyte(0X20);
	SPI1_sendbyte(sectoraddress>>16);
	SPI1_sendbyte(sectoraddress>>8);
	SPI1_sendbyte(sectoraddress);
	PBout(14) = 1;
	uint16_t counter = 0;
	while(1)
	{
		uint8_t res = W25Q128_readstatus1();
		if(0 == (res&0x01))
		{
		  break;
		}
		else
		{
		  counter++;
			DELAY_ms(1);
			if(counter>10000)
			{
			   break;
			}
		}
	}
  W25Q128_writedisable();
}

void W25Q128_writedata(uint32_t address,uint8_t *buff,uint8_t length)
{
	W25Q128_erasesector(5000);
	W25Q128_writeenable();
	PBout(14) = 0;
	SPI1_sendbyte(0x02);
	SPI1_sendbyte(address>>16);
	SPI1_sendbyte(address>>8);
	SPI1_sendbyte(address);
	while(length--)
	{
		SPI1_sendbyte(*buff++);
	}
	PBout(14) = 1;
	uint16_t counter = 0;
	while(1)
	{
		uint8_t res = W25Q128_readstatus1();
		if(0 == (res&0x01))
		{
		  break;
		}
		else
		{
		  counter++;
			DELAY_ms(1);
			if(counter>10000)
			{
			   break;
			}
		}
	}
	W25Q128_writedisable();
}

void W25Q128_readdata(uint32_t address,uint8_t *buff,uint8_t length)
{
	PBout(14) = 0;
	SPI1_sendbyte(0x03);
	SPI1_sendbyte(address>>16);
	SPI1_sendbyte(address>>8);
	SPI1_sendbyte(address);
	while(length--)
	{
		*buff++ = SPI1_sendbyte(0x17);
	}
	uint16_t counter = 0;
	while(1)
	{
		uint8_t res = W25Q128_readstatus1();
		if(0 == (res&0x01))
		{
		  break;
		}
		else
		{
		  counter++;
			DELAY_ms(1);
			if(counter>10000)
			{
			   break;
			}
		}
	}
	PBout(14) = 1;
}
