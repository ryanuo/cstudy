#include "stm32f4xx.h"                  // Device header
#include "ONEWIREBUS.h"
#include "DELAY.h"
#include "BITBAND.h"

//2）封装GPIO灵活输入输出的函数
static void DHT11_setmode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = mode;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_9;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOG,&GPIO_INSTRUCT);
}
//1）使能GPIO口的时钟
void DHT11_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);
	DHT11_setmode(GPIO_Mode_OUT);//默认低电平
	//GPIO_SetBits(GPIOG,GPIO_Pin_9);//拉高总线才是空闲
	PGout(9) = 1;
}

//3）启动DHT11函数
static int8_t DHT11_start(void)
{
	DHT11_setmode(GPIO_Mode_OUT);
	//GPIO_SetBits(GPIOG,GPIO_Pin_9);
	PGout(9) = 1;
	//GPIO_ResetBits(GPIOG,GPIO_Pin_9);
	PGout(9) = 0;
	DELAY_ms(20);
	//GPIO_SetBits(GPIOG,GPIO_Pin_9);
	PGout(9) = 1;
	DELAY_us(30);
	DHT11_setmode(GPIO_Mode_IN);
	uint16_t counter = 0;
	//while(1 == GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_9))
	while(1 == PGin(9))
	{
	  counter++;
		DELAY_us(1);
		if(counter > 1000)//1ms
		{
		  return -1;
		}
	}
	
	counter = 0;
	//while(0 == GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_9))
	while(0 == PGin(9))
	{
		counter++;
		DELAY_us(1);
		if(counter > 100)//0.1ms
		{
		  return -2;
		}
	}
	
	counter = 0;
	//while(1 == GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_9))
	while(1 == PGin(9))
	{
		counter++;
		DELAY_us(1);
		if(counter > 100)//0.1ms
		{
		  return -3;
		}
	}
	return 0;
}
//4）读取单个字节的函数
static uint8_t DHT11_readbyte(void)
{
	uint8_t data = 0;//0000 0000
  for(int i = 0;i<8;i++)
	{
		//while(0 == GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_9));//低+高
		while(0 == PGin(9));
		DELAY_us(50);
		//if(1 == GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_9))
		if(1 == PGin(9))
		{
		  data |= (1<<(7-i));
		}
		//while(1 == GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_9));
		while(1 == PGin(9));
	}
	return data;
}

//5）读取温湿度数据的函数
int8_t DHT11_gettemphum(uint8_t *buff)
{
  int8_t result = DHT11_start();
	if(result < 0)
	{
	 return result;
	}
	for(int i = 0;i<5;i++)
	{
		buff[i] = DHT11_readbyte();
	}
	uint8_t cheaksum = 0;
	cheaksum = buff[0] + buff[1]+ buff[2] +  buff[3];
	if(cheaksum != buff[4])
	{
	 return -4;	
	}
	return 0;
}
