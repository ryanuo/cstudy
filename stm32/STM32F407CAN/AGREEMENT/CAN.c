#include "stm32f4xx.h"                  // Device header
#include "CAN.h"

void CAN_init(void)
{
//1）使能GPIOD组的时钟
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
//2）GPIO结构体配置
GPIO_InitTypeDef GPIO_INSTRUCT;
GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_AF;
GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1;
GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
//3）GPIO配置生效
GPIO_Init(GPIOD,&GPIO_INSTRUCT);
//4）将GPIO口复用为CAN口
GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_CAN1);
GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_CAN1);
//5）配置CAN的结构体
RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1,ENABLE);
CAN_InitTypeDef CAN_INSTRUCT;
CAN_INSTRUCT.CAN_Mode = CAN_Mode_Normal;//正常模式，不选择回路模式
CAN_INSTRUCT.CAN_ABOM = ENABLE;//置1 开启离线自动恢复 置0关闭离线自动恢复
CAN_INSTRUCT.CAN_AWUM = DISABLE;//置1 自动唤醒 一旦监测到CAN上有硬件活动自动清零Sleep  置0手动唤醒，软件代码清零Sleep
CAN_INSTRUCT.CAN_NART = DISABLE;//置1 关闭自动重传功能 CAN报文只能发送一次 置0 开启自动重传功能 CAN报文发送成功为止
CAN_INSTRUCT.CAN_RFLM = DISABLE;//置1 接收FIFO绑定 FIFO溢出时，新收到的报文会被自动丢弃 置0FIFO产生溢出时，最后一次新报文会覆盖就报文
CAN_INSTRUCT.CAN_TTCM = DISABLE;//置1开启通讯触发功能 置0关闭通讯触发功能
CAN_INSTRUCT.CAN_TXFP = DISABLE;//置1优先级根据发送顺序来决定 置0按照ID大小发送，ID越小优先级越高
//CAN总线通讯波特率= 42MHZ/CAN_Prescaler/(BS1+BS2+SJW) = 500K
CAN_INSTRUCT.CAN_BS1 =CAN_BS1_11tq;//11
CAN_INSTRUCT.CAN_BS2 = CAN_BS2_2tq;//2
CAN_INSTRUCT.CAN_Prescaler  = 6;//6
CAN_INSTRUCT.CAN_SJW = CAN_SJW_1tq;//1  SJW的值不能大于BS2
//6）CAN结构体配置生效
CAN_Init(CAN1,&CAN_INSTRUCT);
//7）配置CAN的滤波器---->不对数据进行过滤
CAN_FilterInitTypeDef CAN_FILTERINSTRUCT;
CAN_FILTERINSTRUCT.CAN_FilterActivation = ENABLE;//使能过滤器0
CAN_FILTERINSTRUCT.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;//匹配的报文存入FIFE0
CAN_FILTERINSTRUCT.CAN_FilterIdHigh = 0x0000;//过滤器的ID高16位
CAN_FILTERINSTRUCT.CAN_FilterIdLow = 0x0000;//过滤器的ID低16位
CAN_FILTERINSTRUCT.CAN_FilterMaskIdHigh = 0x0000;//掩码高16位 0不校验
CAN_FILTERINSTRUCT.CAN_FilterMaskIdLow = 0x0000;//掩码低16位 0不校验
CAN_FILTERINSTRUCT.CAN_FilterMode = CAN_FilterMode_IdMask;//ID+掩码模式
CAN_FILTERINSTRUCT.CAN_FilterNumber = 0;//选择第0号过滤器
CAN_FILTERINSTRUCT.CAN_FilterScale = CAN_FilterScale_32bit;//32位过滤器模式
//8）CAN滤波器配置生效
CAN_FilterInit(&CAN_FILTERINSTRUCT);
//9）开启CAN的中断:CAN1 FIFO0报文挂起中断 收到报文触发中断
CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);
//10）NVIC结构体配置
NVIC_InitTypeDef NVIC_INSTRUCT;
NVIC_INSTRUCT.NVIC_IRQChannel = CAN1_RX0_IRQn;
NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 1;
NVIC_INSTRUCT.NVIC_IRQChannelSubPriority = 1;
//11）NVIC结构体配置生效
NVIC_Init(&NVIC_INSTRUCT);
}
uint8_t CAN_rxbuff[8] = {0};
uint16_t CAN_rxid = 0;
uint8_t CAN_rxlength = 0;
uint8_t CAN_rxflag = 0;

void CAN1_RX0_IRQHandler(void)
{
 if(CAN_GetITStatus(CAN1,CAN_IT_FMP0) == SET)
 {
   CanRxMsg rxMsg;
	 CAN_Receive(CAN1,CAN_FIFO0,&rxMsg);
	 CAN_rxid = rxMsg.StdId;
	 CAN_rxlength =  rxMsg.DLC;
	 for(int i = 0;i<CAN_rxlength;i++)
	 {
	   CAN_rxbuff[i] = rxMsg.Data[i];
	 }
	 CAN_rxflag = 1;
	 CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0);
 }
}

uint8_t CAN_sendmessage(uint16_t stdid,uint8_t *pdata,uint8_t length)
{
  CanTxMsg txMsg;
	txMsg.DLC = length;//数据长度
	txMsg.IDE = CAN_ID_STD;//标准帧标志
	txMsg.RTR = CAN_RTR_Data;//数据帧 不要远程帧
	txMsg.StdId = stdid;//标准ID
	for(int i = 0;i<length;i++)
	{
   txMsg.Data[i] = 	pdata[i];
	}
	return CAN_Transmit(CAN1,&txMsg);
}