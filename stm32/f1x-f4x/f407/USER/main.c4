#include "stm32f4xx.h"
#include "OLED.h"
#include "LED.h"
#include "USART.h"
#include "ADC.h"         // PA5 / ADC1 滑动变阻器
#include "LIGHTSENSOR.h" // PF7 / ADC3 光敏传感器
#include "INOUT.h"
#include <stdio.h>

/* DAC / ADC 回环测试函数声明（若 INOUT.h 里已声明可删掉这两行） */
void DAC_SetVoltage(float voltage);
float ADC_ReadVoltage(void);

// 简易延时
static void Delay_ms(uint32_t ms)
{
    volatile uint32_t count = ms * 16800; // 适配 168MHz
    while (count--)
    {
    }
}

int main(void)
{
    char buf[24]; /* 用于 sprintf 拼字符串 */

    float set_voltage = 3.3f;
    float read_voltage = 0.0f;
    float error = 0.0f;

    /* 初始化外设 */
    OLED_Init();
    LED_init();
    USART1_init();
    ADC1PA5_Init();
    LIGHT_Init();
    INOUT_Init(); /* DAC + PA6 ADC 初始化，只调用一次 */

    /* OLED 静态标题（只刷一次） */
    OLED_ShowString(0, 0, "DAC-ADC Loop", OLED_8X16);
    OLED_Update(); /* 若你的库更新函数叫别的名字，改成对应名字 */

    while (1)
    {
        /* 1. 设置 DAC 输出电压 */
        DAC_SetVoltage(set_voltage);

        /* 等待 DAC 输出稳定 */
        for (volatile int i = 0; i < 100000; i++)
            ;

        /* 2. 用 ADC 采集 PA6 上的电压 */
        read_voltage = ADC_ReadVoltage();

        /* 3. 计算误差 */
        error = read_voltage - set_voltage;

        /* 4. OLED 显示：用 sprintf 拼好字符串再显示 */
        /* 第 2 行：设定电压 */
        sprintf(buf, "SET:%5.3fV", set_voltage);
        OLED_ShowString(0, 16, buf, OLED_8X16);

        /* 第 3 行：采集电压 */
        sprintf(buf, "RD :%5.3fV", read_voltage);
        OLED_ShowString(0, 32, buf, OLED_8X16);

        /* 第 4 行：误差 */
        sprintf(buf, "ERR:%5.3fV", error);
        OLED_ShowString(0, 48, buf, OLED_8X16);

        /* 5. 刷新到屏幕 */
        OLED_Update();

        /* 6. 改变设定电压，用于循环测试 */
        set_voltage += 0.1f;
        if (set_voltage > 3.3f)
        {
            set_voltage = 0.0f;
        }

        /* 延时约 1 秒（假设 168MHz 主频） */
        for (volatile int i = 0; i < 16800000; i++)
            ;
    }
}