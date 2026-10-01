#include "stm32f4xx.h"                  // Device header
#include "ADC.h"
volatile uint32_t ADC_convalue = 0;//存储器
#if 0
void ADC1PA5_init(void)
{
//1）使能GPIOA组的时钟
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
//2）GPIO配置--模拟输入
GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_5;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
//3）使GPIO配置生效
GPIO_Init(GPIOA,&GPIO_INSTRUCT);
//4）使能ADC外设的时钟(ADC1,ADC2)
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
//5）ADC通用配置
ADC_CommonInitTypeDef ADC_COMMONSTRUCT;
ADC_COMMONSTRUCT.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;//禁用DMA
ADC_COMMONSTRUCT.ADC_Mode = ADC_Mode_Independent;//ADC独立模式
ADC_COMMONSTRUCT.ADC_Prescaler = ADC_Prescaler_Div6;//84M/6=14M
ADC_COMMONSTRUCT.ADC_TwoSamplingDelay =ADC_TwoSamplingDelay_5Cycles;//第一个(ADC1)和第二个(ADC2)采样间隔，只有在双重/三重生效
//6）ADC通用配置生效
ADC_CommonInit(&ADC_COMMONSTRUCT);
//7）ADC配置
ADC_InitTypeDef ADC_INSTRUCT;
ADC_INSTRUCT.ADC_ContinuousConvMode = DISABLE;//单次
ADC_INSTRUCT.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
ADC_INSTRUCT.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;//任意配置
ADC_INSTRUCT.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//不使用外部触发
ADC_INSTRUCT.ADC_NbrOfConversion = 1;//设置规则组通道数量为1
ADC_INSTRUCT.ADC_Resolution = ADC_Resolution_12b;//转换精度/分辨率
ADC_INSTRUCT.ADC_ScanConvMode = DISABLE;//非扫描
//8）ADC配置生效
ADC_Init(ADC1,&ADC_INSTRUCT);
//9）ADC转换(ADC规则或者注入组，转换周期)  总= 采样+转换=3+12=15   15/14 = 1.07us
ADC_RegularChannelConfig(ADC1,ADC_Channel_5,1,ADC_SampleTime_3Cycles);//
//10）开启ADC转换
ADC_Cmd(ADC1,ENABLE);
}

//11）ADC的读数
uint16_t ADC1PA5_getvalue(void)
{
  ADC_SoftwareStartConv(ADC1);
	while(RESET == ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC1);
}
#endif

#if 1
void ADC1PA5_init(void)
{
	//1、开启外设时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
	//2、定义所有的结构体
	GPIO_InitTypeDef GPIO_INSTRUCT;
	ADC_CommonInitTypeDef ADC_COMMONSTRUCT;
	ADC_InitTypeDef ADC_INSTRUCT;
	DMA_InitTypeDef DMA_INSTRUCT;
	//3、GPIO结构体配置
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_5;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	//4、使GPIO配置生效
	GPIO_Init(GPIOA,&GPIO_INSTRUCT);
	//5、ADC结构体配置
	ADC_COMMONSTRUCT.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;//禁用多重DMA
	ADC_COMMONSTRUCT.ADC_Mode = ADC_Mode_Independent;//ADC独立模式
	ADC_COMMONSTRUCT.ADC_Prescaler = ADC_Prescaler_Div6;//84M/6=14M
	ADC_COMMONSTRUCT.ADC_TwoSamplingDelay =ADC_TwoSamplingDelay_5Cycles;//第一个(ADC1)和第二个(ADC2)采样间隔，只有在双重/三重生效
	//6、ADC通用配置生效
	ADC_CommonInit(&ADC_COMMONSTRUCT);
	//7、ADC配置
	ADC_INSTRUCT.ADC_ContinuousConvMode = ENABLE;//连续
	ADC_INSTRUCT.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
	ADC_INSTRUCT.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;//任意配置
	ADC_INSTRUCT.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//不使用外部触发
	ADC_INSTRUCT.ADC_NbrOfConversion = 1;//设置规则组通道数量为1
	ADC_INSTRUCT.ADC_Resolution = ADC_Resolution_12b;//转换精度/分辨率
	ADC_INSTRUCT.ADC_ScanConvMode = DISABLE;//非扫描
	//8、ADC配置生效
	ADC_Init(ADC1,&ADC_INSTRUCT);
	//9、ADC转换(ADC规则或者注入组，转换周期)  总= 采样+转换=3+12=15   15/14 = 1.07us
	ADC_RegularChannelConfig(ADC1,ADC_Channel_5,1,ADC_SampleTime_3Cycles);//
	//10、开启ADC转换
	ADC_Cmd(ADC1,ENABLE);
	//11、DMA结构体配置
	DMA_INSTRUCT.DMA_BufferSize = 1;//DMA一次数据的个数
	DMA_INSTRUCT.DMA_Channel = DMA_Channel_0;//查看DMA2表格
	DMA_INSTRUCT.DMA_DIR = DMA_DIR_PeripheralToMemory;//外设--->存储器
	DMA_INSTRUCT.DMA_FIFOMode = DMA_FIFOMode_Disable;//禁用FIFO缓冲区
	DMA_INSTRUCT.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;//1/4
	DMA_INSTRUCT.DMA_Memory0BaseAddr = (uint32_t)&ADC_convalue;//存储器地址
	DMA_INSTRUCT.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器单次传输1个数据
	DMA_INSTRUCT.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;//存储器数据大小半字，字接收
	DMA_INSTRUCT.DMA_MemoryInc = DMA_MemoryInc_Disable;//存储器地址不自增
	DMA_INSTRUCT.DMA_Mode = DMA_Mode_Circular;//DMA循环模式
	DMA_INSTRUCT.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;//外设基地址
	DMA_INSTRUCT.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设单次传输1个数据
	DMA_INSTRUCT.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;//字发送
	DMA_INSTRUCT.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不自增
	DMA_INSTRUCT.DMA_Priority = DMA_Priority_High;//高优先级
	//12、DMA配置生效
	DMA_Init(DMA2_Stream0,&DMA_INSTRUCT);
	//13、启动DMA
	DMA_Cmd(DMA2_Stream0,ENABLE);
	//14、将ADC与DMA连接起来
  ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE);//防止DMA转运一次ADC
	ADC_DMACmd(ADC1,ENABLE);
	ADC_SoftwareStartConv(ADC1);
}
#endif
