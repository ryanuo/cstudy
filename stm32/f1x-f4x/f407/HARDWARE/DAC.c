#include "stm32f4xx.h"

void DAC1_Init(void)
{
    // 1. 使能 GPIOA 和 DAC 外设时钟
    // DAC 挂载在 APB1 总线上
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    // 2. 配置 PA4 为模拟输入模式 (GPIO_Mode_AN)
    // 官方规定：即使引脚用于 DAC 模拟输出，GPIO 也必须配置为模拟模式以关闭内部数字施密特触发器
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. DAC 通道 1 初始化配置
    DAC_InitTypeDef DAC_InitStructure;
    // 软件触发模式：由软件写入寄存器直接触发转换，无需定时器/外部中断
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
    // 关闭波形发生器（输出固定直流电平时关闭）
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
    // 开启输出缓冲区 (Output Buffer)：增强引脚的驱动能力，降低输出阻抗
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);

    // 4. 使能 DAC 通道 1
    DAC_Cmd(DAC_Channel_1, ENABLE);

    // 5. 初始默认输出 0V
    DAC_SetChannel1Data(DAC_Align_12b_R, 0);
}

/**
 * @brief  向 DAC1 (PA4) 写入 12 位原始码值
 * @param  value: 0 ~ 4095
 */
void DAC1_SetRawValue(uint16_t value)
{
    if (value > 4095)
        value = 4095;
    // 写入 12 位右对齐数据保持寄存器
    DAC_SetChannel1Data(DAC_Align_12b_R, value);
}

/**
 * @brief  让 DAC1 (PA4) 输出指定电压
 * @param  voltage: 目标电压值 (0.0f ~ 3.3f)
 */
void DAC1_SetVoltage(float voltage)
{
    if (voltage < 0.0f)
        voltage = 0.0f;
    if (voltage > 3.3f)
        voltage = 3.3f;

    // 换算公式: Code = (Voltage / 3.3V) * 4095
    uint16_t code = (uint16_t)((voltage / 3.3f) * 4095.0f);
    DAC_SetChannel1Data(DAC_Align_12b_R, code);
}