#include "stm32f4xx.h"                  // Device header
#include "USART.h"
#include "LED.h"
#include <stdio.h>
void USART1_init(void)
{
//1、时钟使能GPIOA组的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
//2、设置脚位功能，复用推挽输出TX，上拉输入
	GPIO_InitTypeDef GPIO_INSTRUCT;	
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_AF;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
//3、使GPIO配置生效
	GPIO_Init(GPIOA,&GPIO_INSTRUCT);
//4、使能USART1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
//5、将GPIO复用为串口
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
//6、USART结构体配置
	USART_InitTypeDef USART1_INSTRUCT;
	USART1_INSTRUCT.USART_BaudRate = 9600;//一定要保证波特率一致
	USART1_INSTRUCT.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不流控
	USART1_INSTRUCT.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;//收发模式
	USART1_INSTRUCT.USART_Parity = USART_Parity_No;//CRC函数进行校验
	USART1_INSTRUCT.USART_StopBits = USART_StopBits_1;//停止位
	USART1_INSTRUCT.USART_WordLength = USART_WordLength_8b;//字长  5-9bit
//7、使USART1配置生效
  USART_Init(USART1,&USART1_INSTRUCT);
//8、启动串口接收中断
USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
//9、NVIC结构体配置
  NVIC_InitTypeDef NVIC_INSTRUCT;
	NVIC_INSTRUCT.NVIC_IRQChannel = USART1_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority = 2;
//10、使NVIC配置生效
  NVIC_Init(&NVIC_INSTRUCT);
//11、启动串口
  USART_Cmd(USART1,ENABLE);
}

//12、提供中断服务函数
void USART1_IRQHandler(void)
{
	//1）获取中断标志位
  if(SET == USART_GetITStatus(USART1,USART_IT_RXNE))
	{
	   uint8_t DATA = USART_ReceiveData(USART1);
		if(DATA == 0XAA)
		{
		  USART_SendData(USART1,0XCC);
			LED1_on();
		}
		if(DATA == 0XBB)
		{
			USART_SendData(USART1,0XDD);
			LED1_off();
		}
		//3）清除中断标志位
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
}

void USART1_SendData(uint8_t data)//最基础保证字节单个发送
{
	USART_SendData(USART1,data);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}

void USART1_SendString(char*string)//AT指令集,MQTT,TCP
{
	for(int i = 0;string[i] !='\0';i++)
	{		
		USART1_SendData(string[i]);
	}
}

//printf重定向
int fputc(int ch,FILE*f)
{
	USART1_SendData(ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	return ch;
}	
