#include "stm32f4xx.h"                  // Device header
#include "IIC.h"
#include "BITBAND.h"
#include "DELAY.h"

static void IIC_setmode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = mode;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_OD;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_9;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB,&GPIO_INSTRUCT);
}

void IICSOFTWARE_init(void)
{
//1）使能GPIOB组的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
//2）配置GPIO的SCL为开漏输出，灵活配置GPIO的SDA输入输出
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_OD;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_8;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
//3）使GPIO配置生效
	GPIO_Init(GPIOB,&GPIO_INSTRUCT);
	IIC_setmode(GPIO_Mode_OUT);
	PBout(8) = 1;
	PBout(9) = 1;
}

//4）IIC启动
void IIC_start(void)//100KHZ
{
	IIC_setmode(GPIO_Mode_OUT);
	PBout(8) = 1;
	PBout(9) = 1;
	DELAY_us(5);
	PBout(9) = 0;
	DELAY_us(5);
	PBout(8) = 0;
	DELAY_us(5);
}
//5）IIC结束
void IIC_stop(void)
{
	IIC_setmode(GPIO_Mode_OUT);
	PBout(8) = 0;
	PBout(9) = 0;
	DELAY_us(5);
	PBout(8) = 1;
	DELAY_us(5);
	PBout(9) = 1;
	DELAY_us(5);
}
//6）主机向从机发送一个字节
void IIC_sendbyte(uint8_t byte)//0XAA
{
	IIC_setmode(GPIO_Mode_OUT);
	PBout(8) = 0;
	PBout(9) = 0;
	for(int i = 0;i<8;i++)
	{
	  if(byte&(1<<(7-i)))
			PBout(9) = 1;
		else
			PBout(9) = 0;
		DELAY_us(5);
		
		PBout(8) = 1;
		DELAY_us(5);
		PBout(8) = 0;
		DELAY_us(5);
	}
}
//7）从机向主机发送一个bit
uint8_t IIC_waitack(void)
{
	uint8_t ack = 0;
	IIC_setmode(GPIO_Mode_IN);
	PBout(8) = 1;
	DELAY_us(5);
	if(PBin(9) == 0)//低电平----有应答   高电平-----无应答
	{
	 ack = 1;
	}
	else
	{
	 ack = 0;
	}
	PBout(8) = 0;
	DELAY_us(5);
	return ack;
}
//8）从机向主机发送一个字节
uint8_t IIC_revicebyte(void)
{
	uint8_t data = 0;
	IIC_setmode(GPIO_Mode_IN);
	for(int i = 0;i<8;i++)
	{
		PBout(8) = 1;
		DELAY_us(5);
		if(PBin(9) == 1)
		{
		 data |= (1<<(7-i));
		}
		PBout(8) = 0;
		DELAY_us(5);
	}
	return data;
}

//9）主机向从机发送一个bit
void IIC_sendack(uint8_t ack)//0XAA
{
	IIC_setmode(GPIO_Mode_OUT);
	PBout(8) = 0;
	PBout(9) = 0;
  if(ack)
	{
	  	PBout(9) = 0;
	}
	else
	{
			PBout(9) = 1;
	}
		PBout(8) = 1;
		DELAY_us(5);
		PBout(8) = 0;
		DELAY_us(5);
}




