#include "stm32f4xx.h"                  // Device header
#include "DAC.h"

void DACOUT1_init(void)
{
//1）使能GPIOA组的时钟
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
//2）GPIO配置--模拟输入
GPIO_InitTypeDef GPIO_INSTRUCT;
GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_AIN;
GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_4;
GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
//3）使GPIO配置生效
GPIO_Init(GPIOA,&GPIO_INSTRUCT);
//4）使能DAC的时钟
RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC,ENABLE);
//5）配置DAC的结构体
DAC_InitTypeDef DAC_INSTRUCT;
DAC_INSTRUCT.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;//三角波和噪声波
DAC_INSTRUCT.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
DAC_INSTRUCT.DAC_Trigger = DAC_Trigger_None;
DAC_INSTRUCT.DAC_WaveGeneration = DAC_WaveGeneration_None;
//6）使DAC配置生效
DAC_Init(DAC_Channel_1,&DAC_INSTRUCT);
//7）启动DAC
DAC_Cmd(DAC_Channel_1,ENABLE);
}
