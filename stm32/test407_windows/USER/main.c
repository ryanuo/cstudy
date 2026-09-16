#include "stm32f4xx.h" // Device header
#include "OLED.h"
#include "LED.h"
#include "USART.h"

int main(void)
{
    OLED_Init();
    LED_init();

    USART1_init();
    char buf[20];

    while (1)
    {
        USART1_SendData(0XBB);
        DELAY_ms(1000);
        USART1_SendString("AT+NAME\r\n");
        DELAY_ms(1000);
        printf("hello world\r\n");
        DELAY_ms(1000);
        // if (USART_flag == 1)
        // {
        //     LED1_on();
        //     OLED_ShowString(0, 0, "OXAA", OLED_8X16);
        // }

        // if (USART_flag == 2)
        // {
        //     LED1_off();
        //     OLED_ShowString(0, 0, "OX55", OLED_8X16);
        // }
        OLED_Update();
    }
}