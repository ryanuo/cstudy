#include "stm32f10x.h" // Device header
#include <stdio.h>
#include <stdarg.h>

uint8_t Serial_TxPacket[4]; // 定义发送数据包数组，数据包格式：FF 01 02 03 04 CRC_L CRC_H FE
uint8_t Serial_RxPacket[4]; // 定义接收数据包数组
uint8_t Serial_RxFlag;		// 定义接收数据包标志位
uint16_t Serial_RxCrc;		// 定义接收到的CRC-16值

/**
 * 函    数：CRC-16/MODBUS 计算函数
 * 参    数：Data   数据首地址
 * 参    数：Length 数据长度（字节）
 * 返 回 值：16位CRC值
 * 说    明：多项式 0x8005（反射形式 0xA001），初值 0xFFFF
 */
uint16_t Serial_CRC16(uint8_t *Data, uint16_t Length)
{
	uint16_t crc = 0xFFFF; // 初始值
	uint16_t i;
	uint8_t j;
	for (i = 0; i < Length; i++)
	{
		crc ^= Data[i]; // 与当前字节异或（低位）
		for (j = 0; j < 8; j++)
		{
			if (crc & 0x0001) // 判断最低位
			{
				crc = (crc >> 1) ^ 0xA001; // 右移后异或多项式（反射形式）
			}
			else
			{
				crc = (crc >> 1);
			}
		}
	}
	return crc;
}

/**
 * 函    数：串口初始化（保持不变）
 */
void Serial_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*USART初始化*/
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);

	/*中断输出配置*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

	/*USART使能*/
	USART_Cmd(USART1, ENABLE);
}

/* ===== 以下发送相关函数保持不变 ===== */

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
		;
}

void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i++)
	{
		Serial_SendByte(Array[i]);
	}
}

void Serial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		Serial_SendByte(String[i]);
	}
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)
	{
		Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
	}
}

int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;
}

void Serial_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serial_SendString(String);
}

/**
 * 函    数：串口发送数据包（已加入 CRC-16/MODBUS）
 * 说    明：数据包格式：FF [4字节数据] [CRC_L] [CRC_H] FE
 */
void Serial_SendPacket(void)
{
	uint16_t crc = Serial_CRC16(Serial_TxPacket, 4); // 计算数据的CRC-16
	Serial_SendByte(0xFF);
	Serial_SendArray(Serial_TxPacket, 4);
	Serial_SendByte(crc & 0xFF);		// CRC 低字节在前
	Serial_SendByte((crc >> 8) & 0xFF); // CRC 高字节在后
	Serial_SendByte(0xFE);
}

/**
 * 函    数：获取串口接收数据包标志位（保持不变）
 */
uint8_t Serial_GetRxFlag(void)
{
	if (Serial_RxFlag == 1)
	{
		Serial_RxFlag = 0;
		return 1;
	}
	return 0;
}

/**
 * 函    数：USART1中断函数（已加入 CRC-16 校验）
 * 说    明：状态机流程：
 *           状态0：等待包头 0xFF
 *           状态1：接收 4 字节数据
 *           状态2：接收 CRC 低字节
 *           状态3：接收 CRC 高字节
 *           状态4：接收包尾 0xFE，并校验 CRC-16
 */
void USART1_IRQHandler(void)
{
	static uint8_t RxState = 0;	  // 当前状态机状态
	static uint8_t pRxPacket = 0; // 当前接收数据位置
	static uint8_t RxCrcLow = 0;  // CRC 低字节暂存
	static uint8_t RxCrcHigh = 0; // CRC 高字节暂存

	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		uint8_t RxData = USART_ReceiveData(USART1);

		/*状态0：接收包头*/
		if (RxState == 0)
		{
			if (RxData == 0xFF)
			{
				RxState = 1;
				pRxPacket = 0;
			}
		}
		/*状态1：接收4字节数据*/
		else if (RxState == 1)
		{
			Serial_RxPacket[pRxPacket] = RxData;
			pRxPacket++;
			if (pRxPacket >= 4)
			{
				RxState = 2;
			}
		}
		/*状态2：接收CRC低字节*/
		else if (RxState == 2)
		{
			RxCrcLow = RxData;
			RxState = 3;
		}
		/*状态3：接收CRC高字节*/
		else if (RxState == 3)
		{
			RxCrcHigh = RxData;
			RxState = 4;
		}
		/*状态4：接收包尾并校验*/
		else if (RxState == 4)
		{
			if (RxData == 0xFE) // 包尾正确
			{
				uint16_t recvCrc = RxCrcLow | (RxCrcHigh << 8);		 // 拼接收到的CRC
				uint16_t calcCrc = Serial_CRC16(Serial_RxPacket, 4); // 计算实际CRC
				if (calcCrc == recvCrc)								 // CRC校验通过
				{
					Serial_RxCrc = recvCrc;
					Serial_RxFlag = 1; // 接收成功
				}
				/* CRC不匹配则不置标志位，丢弃该包 */
			}
			RxState = 0; // 无论成功失败都回到状态0
		}

		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}