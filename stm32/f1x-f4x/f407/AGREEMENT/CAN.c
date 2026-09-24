#include "CAN.h"

void CAN_init()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    /* 配置 GPIO 为复用功能（必须配 OType 和 PuPd） */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   // 上拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_CAN1);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_CAN1);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    CAN_InitTypeDef CAN_InitStructure;
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; // Set the CAN mode
    CAN_InitStructure.CAN_ABOM = ENABLE;          // Enable automatic bus-off management
    CAN_InitStructure.CAN_AWUM = DISABLE;         // Enable automatic wake-up mode
    CAN_InitStructure.CAN_NART = DISABLE;         // Disable automatic retransmission
    CAN_InitStructure.CAN_RFLM = DISABLE;         // Disable receive FIFO locked mode
    CAN_InitStructure.CAN_TTCM = DISABLE;         // Disable time-triggered communication mode
    CAN_InitStructure.CAN_TXFP = DISABLE;         // Disable transmit FIFO priority
    // 配置波特率 公式：BaudRate = Fpclk1 / (CAN_BS1 + CAN_BS2) * CAN_Prescaler 500k
    CAN_InitStructure.CAN_Prescaler = 6;      // Set the prescaler value
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;  // Set the synchronization jump width
    CAN_InitStructure.CAN_BS1 = CAN_BS1_11tq; // Set the time segment 1
    CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;  // Set the time segment 2
    CAN_Init(CAN1, &CAN_InitStructure);

    CAN_FilterInitTypeDef CAN_FilterInitStructure;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInit(&CAN_FilterInitStructure);
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE); // Enable FIFO 0 message pending interrupt

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
}

void CAN1_RX0_IRQHandler(void)
{
    if (CAN_GetFlagStatus(CAN1, CAN_FLAG_FMP0) != RESET) // Check if there is a message pending in FIFO 0
    {
        CanRxMsg RxMessage;
        CAN_Receive(CAN1, CAN_FIFO0, &RxMessage); // Receive the message
        // Process the received message (RxMessage)
        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    }
}

uint8_t CAN_send_message(uint32_t id, uint8_t *data, uint8_t length)
{
    CanTxMsg TxMessage;
    TxMessage.StdId = id;            // Set the standard identifier
    TxMessage.IDE = CAN_Id_Standard; // Standard identifier
    TxMessage.RTR = CAN_RTR_Data;    // Data frame
    TxMessage.DLC = length;          // Data length code
    for (uint8_t i = 0; i < length; i++)
    {
        TxMessage.Data[i] = data[i]; // Copy data to the message
    }

    uint8_t mailbox = CAN_Transmit(CAN1, &TxMessage); // Transmit the message
    return mailbox;                                   // Return the mailbox number used for transmission
}