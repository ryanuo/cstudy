#include "stm32f4xx.h"
#include <stdio.h>
#include <stdarg.h>

/* ===================== 全局变量 ===================== */
volatile uint8_t USART_flag = 0;      // USART1 收到的指令标志

uint8_t  Serial_RxPacket[4];          // USART2 接收的数据包
uint8_t  Serial_RxFlag = 0;           // USART2 收到完整包标志
uint16_t Serial_RxCrc = 0;            // 接收到的 CRC-16

/* ===================== CRC-16/MODBUS ===================== */
uint16_t Serial_CRC16(uint8_t *Data, uint16_t Length)
{
    uint16_t crc = 0xFFFF;
    uint16_t i;
    uint8_t  j;
    for (i = 0; i < Length; i ++)
    {
        crc ^= Data[i];
        for (j = 0; j < 8; j ++)
        {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }
    }
    return crc;
}

/* ============================================================
 *                       USART1（接上位机）
 * ============================================================ */
void USART1_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,  GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

void USART1_SendData(uint8_t data)
{
    USART_SendData(USART1, data);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void USART1_SendString(char *string)
{
    while (*string != '\0')
    {
        USART1_SendData((uint8_t)*string);
        string++;
    }
}

void USART1_Printf(char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    USART1_SendString(String);
}

int fputc(int ch, FILE *f)
{
    USART1_SendData((uint8_t)ch);
    return ch;
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        uint8_t c = (uint8_t)USART_ReceiveData(USART1);

        if (c == 0xAA)
        {
            USART_flag = 1;
            USART1_SendData(0xCC);
        }
        else if (c == 0x55)
        {
            USART_flag = 2;
            USART1_SendData(0xDD);
        }

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

/* ============================================================
 *                    USART2（接 F103，收 CRC-16 包）
 * ============================================================ */
void USART2_init(void)
{
    /* 时钟：USART2 在 APB1，GPIOA 在 AHB1 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);   // PA2 = USART2_TX
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);   // PA3 = USART2_RX

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = 9600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;   // 比 USART1 低
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART2, ENABLE);
}

/**
  * 函    数：USART2 中断：CRC-16 状态机
  *           流程：FF → 4字节数据 → CRC低 → CRC高 → FE
  */
void USART2_IRQHandler(void)
{
    static uint8_t RxState   = 0;
    static uint8_t pRxPacket = 0;
    static uint8_t RxCrcLow  = 0;
    static uint8_t RxCrcHigh = 0;

    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        uint8_t RxData = (uint8_t)USART_ReceiveData(USART2);

        /* 状态0：等包头 FF */
        if (RxState == 0)
        {
            if (RxData == 0xFF)
            {
                RxState   = 1;
                pRxPacket = 0;
            }
        }
        /* 状态1：收 4 字节数据 */
        else if (RxState == 1)
        {
            Serial_RxPacket[pRxPacket] = RxData;
            pRxPacket ++;
            if (pRxPacket >= 4)
                RxState = 2;
        }
        /* 状态2：收 CRC 低字节 */
        else if (RxState == 2)
        {
            RxCrcLow = RxData;
            RxState  = 3;
        }
        /* 状态3：收 CRC 高字节 */
        else if (RxState == 3)
        {
            RxCrcHigh = RxData;
            RxState   = 4;
        }
        /* 状态4：收包尾并校验 */
        else if (RxState == 4)
        {
            if (RxData == 0xFE)
            {
                uint16_t recvCrc = RxCrcLow | (RxCrcHigh << 8);
                uint16_t calcCrc = Serial_CRC16(Serial_RxPacket, 4);
                if (calcCrc == recvCrc)
                {
                    Serial_RxCrc  = recvCrc;
                    Serial_RxFlag = 1;          // 校验通过
                }
                /* 校验失败则丢弃 */
            }
            RxState = 0;
        }

        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}