#include "stm32f10x.h"      // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "Key.h"

uint8_t KeyNum;             // 按键键码

int main(void)
{
    /* ==================== 1. 模块初始化 ==================== */
    OLED_Init();
    Key_Init();
    Serial_Init();          // USART1，9600，PA9=TX，PA10=RX

    /* ==================== 2. 静态显示 ==================== */
    OLED_ShowString(1, 1, "TxPacket");
    OLED_ShowString(3, 1, "RxPacket");

    /* ==================== 3. 发送数据初始值 ==================== */
    Serial_TxPacket[0] = 0x01;
    Serial_TxPacket[1] = 0x02;
    Serial_TxPacket[2] = 0x03;
    Serial_TxPacket[3] = 0x04;

    /* 显示初始值 */
    OLED_ShowHexNum(2, 1, Serial_TxPacket[0], 2);
    OLED_ShowHexNum(2, 4, Serial_TxPacket[1], 2);
    OLED_ShowHexNum(2, 7, Serial_TxPacket[2], 2);
    OLED_ShowHexNum(2, 10, Serial_TxPacket[3], 2);

    /* ==================== 4. 主循环 ==================== */
    uint16_t autoTick = 0;      // 自动发送计时（粗略）

    while (1)
    {
        /* ---------- 4.1 按键1：手动发送一次 ---------- */
        KeyNum = Key_GetNum();
        if (KeyNum == 1)
        {
            Serial_TxPacket[0]++;
            Serial_TxPacket[1]++;
            Serial_TxPacket[2]++;
            Serial_TxPacket[3]++;

            Serial_SendPacket();        // 发送带 CRC-16 的数据包

            /* 刷新 OLED 上的发送值 */
            OLED_ShowHexNum(2, 1, Serial_TxPacket[0], 2);
            OLED_ShowHexNum(2, 4, Serial_TxPacket[1], 2);
            OLED_ShowHexNum(2, 7, Serial_TxPacket[2], 2);
            OLED_ShowHexNum(2, 10, Serial_TxPacket[3], 2);
        }

        // /* ---------- 4.2 自动周期发送（约 1 秒一次） ---------- */
        // autoTick++;
        // if (autoTick >= 100)            // 100 × 10ms ≈ 1s
        // {
        //     autoTick = 0;

        //     Serial_TxPacket[0]++;
        //     Serial_TxPacket[1]++;
        //     Serial_TxPacket[2]++;
        //     Serial_TxPacket[3]++;

        //     Serial_SendPacket();        // 自动发送

        //     OLED_ShowHexNum(2, 1, Serial_TxPacket[0], 2);
        //     OLED_ShowHexNum(2, 4, Serial_TxPacket[1], 2);
        //     OLED_ShowHexNum(2, 7, Serial_TxPacket[2], 2);
        //     OLED_ShowHexNum(2, 10, Serial_TxPacket[3], 2);
        // }

        /* ---------- 4.3 接收 F407 回包（如果 F407 有回发） ---------- */
        if (Serial_GetRxFlag() == 1)
        {
            OLED_ShowHexNum(4, 1, Serial_RxPacket[0], 2);
            OLED_ShowHexNum(4, 4, Serial_RxPacket[1], 2);
            OLED_ShowHexNum(4, 7, Serial_RxPacket[2], 2);
            OLED_ShowHexNum(4, 10, Serial_RxPacket[3], 2);
        }

        /* ---------- 4.4 主循环延时 ---------- */
        Delay_ms(10);           // 让循环稳定在约 10ms 一次
    }
}