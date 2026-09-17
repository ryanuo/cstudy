#include "stm32f4xx.h"
#include "OLED.h"
#include "LED.h"
#include "USART.h"
#include "ADC.h"   // PA5 / ADC1 滑动变阻器
#include "LIGHTSENSOR.h" // PF7 / ADC3 光敏传感器

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
    // 1. 各模块初始化
    OLED_Init();
    LED_init();
    USART1_init();
    ADC1PA5_Init();   // 初始化 PA5 与 ADC1
    LIGHT_Init(); // 初始化 PF7 与 ADC3

    while (1)
    {
        // 2. 读取两路传感器 12 位 ADC 值 (0 ~ 4095)
        uint16_t pot_raw = ADC1_getvalue();
        uint16_t light_raw = LIGHT_GetValue();

        // 3. 换算为 0 ~ 100%
        uint8_t set_thresh = (uint8_t)((pot_raw / 4095.0f) * 100.0f);
        uint8_t real_light = (uint8_t)((1.0f - (light_raw / 4095.0f)) * 100.0f);

        // 4. 联动逻辑与指示状态
        char *lamp_status = "OFF";
        if (real_light < (set_thresh > 3 ? set_thresh - 3 : 0))
        {
            LED1_on();
            lamp_status = "ON ";
        }
        else if (real_light > (set_thresh + 3))
        {
            LED1_off();
            lamp_status = "OFF";
        }

        // 5. OLED 绘制（采用像素坐标：8x16 字体每行高 16 像素）
        // Y 轴：第 1 行 Y=0，第 2 行 Y=16，第 3 行 Y=32

        // 若驱动库支持 OLED_Printf（最推荐，格式化占位符直接防止残影）：
        OLED_Printf(0, 0, OLED_8X16, "Set : %3d %%", set_thresh);
        OLED_Printf(0, 16, OLED_8X16, "Real: %3d %%", real_light);
        OLED_Printf(0, 32, OLED_8X16, "Lamp: %s", lamp_status);

        // 6. 关键步骤：将显存缓冲区数据推送到实际物理屏幕！
        // （请核对你的头文件中更新函数名是 OLED_Update 还是 OLED_Refresh）
        OLED_Update();

        // 7. 串口同步输出调试
        USART1_Printf("Set: %u%% | Real: %u%% | Light: %s\r\n", set_thresh, real_light, lamp_status);

        // 刷新间隔 2200ms
        Delay_ms(2200);
    }
}