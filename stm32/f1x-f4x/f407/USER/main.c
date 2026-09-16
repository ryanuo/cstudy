#include "stm32f4xx.h"      // Device header
#include "OLED.h"
#include "LED.h"
#include "USART.h"

int main(void)
{
    /* ==================== 1. 初始化 ==================== */
    OLED_Init();
    LED_init();

    USART1_init();          // 接上位机 PA9/PA10
    USART2_init();          // 接 F103   PA2/PA3

    /* ==================== 2. 开机界面 ==================== */
    OLED_Clear();
    OLED_ShowString(0, 0, "F407 System", OLED_8X16);
    OLED_ShowString(0, 2, "U1: PC",      OLED_8X16);
    OLED_ShowString(0, 4, "U2: F103",    OLED_8X16);
    OLED_ShowString(0, 6, "Wait F103...", OLED_8X16);
    OLED_Update();

    USART1_SendString("F407 Ready, waiting for F103...\r\n");

    /* ==================== 3. 变量 ==================== */
    uint32_t cnt = 0;           // 收到包的计数
    char buf[32];               // 临时格式化缓冲区

    /* ==================== 4. 主循环 ==================== */
    while (1)
    {
        /* ---------- 4.1 收到 F103 一个 CRC 正确的包 ---------- */
        if (Serial_RxFlag == 1)
        {
            Serial_RxFlag = 0;
            cnt++;

            /* ---- 4.1.1 通过 USART1 发到上位机 ---- */
            USART1_Printf("RX: %02X %02X %02X %02X  CRC=%04X\r\n",
                          Serial_RxPacket[0], Serial_RxPacket[1],
                          Serial_RxPacket[2], Serial_RxPacket[3],
                          Serial_RxCrc);

            /* ---- 4.1.2 在 OLED 上显示 ---- */
            OLED_Clear();

            /* 第 0 行：标题 */
            OLED_ShowString(0, 0, "F103->F407 OK ", OLED_8X16);

            /* 第 2 行：D0 D1 */
            OLED_ShowString(0, 2, "D0:", OLED_8X16);
            OLED_ShowHexNum(24, 2, Serial_RxPacket[0], 2, OLED_8X16);
            OLED_ShowString(56, 2, "D1:", OLED_8X16);
            OLED_ShowHexNum(80, 2, Serial_RxPacket[1], 2, OLED_8X16);

            /* 第 4 行：D2 D3 */
            OLED_ShowString(0, 4, "D2:", OLED_8X16);
            OLED_ShowHexNum(24, 4, Serial_RxPacket[2], 2, OLED_8X16);
            OLED_ShowString(56, 4, "D3:", OLED_8X16);
            OLED_ShowHexNum(80, 4, Serial_RxPacket[3], 2, OLED_8X16);

            /* 第 6 行：计数 + CRC */
            OLED_ShowString(0, 6, "N:", OLED_8X16);
            OLED_ShowNum(16, 6, cnt, 5, OLED_8X16);
            OLED_ShowString(64, 6, "C:", OLED_8X16);
            OLED_ShowHexNum(80, 6, Serial_RxCrc, 4, OLED_8X16);

            OLED_Update();      // ★ 必须刷新

            /* ---- 4.1.3 根据数据做 LED 指示（示例） ---- */
            switch (Serial_RxPacket[0])
            {
                case 0x01:
                    LED1_on();
                    break;
                case 0x02:
                    LED1_off();
                    break;
                default:
                    break;
            }
        }

        /* ---------- 4.2 处理上位机发来的指令 ---------- */
        if (USART_flag != 0)
        {
            uint8_t f = USART_flag;
            USART_flag = 0;

            USART1_Printf("PC cmd: %d\r\n", f);

            /* 在 OLED 右上角显示 PC 指令 */
            OLED_ShowString(80, 0, "PC:", OLED_8X16);
            OLED_ShowNum(104, 0, f, 1, OLED_8X16);
            OLED_Update();

            /* 根据 PC 指令做动作 */
            if (f == 1)
            {
                LED1_on();
            }
            else if (f == 2)
            {
                LED1_off();
            }
        }

        // /* ---------- 4.3 周期性向上位机发心跳（可选） ---------- */
        // static uint32_t tick = 0;
        // tick++;
        // if (tick >= 500000)         // 粗略延时，实际可换成 SysTick
        // {
        //     tick = 0;
        //     USART1_SendString("F407 alive\r\n");
        // }
    }
}