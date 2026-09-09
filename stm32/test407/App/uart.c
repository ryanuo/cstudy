#include "uart.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef huart3;

void UART3_Init(void)
{
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &gpio);

    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart3);
}

void UART3_SendChar(char c)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&c, 1, 100);
}

void UART3_SendString(const char *str)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);
}

void UART3_SendCharRaw(char c)
{
    while (!(USART3->SR & USART_SR_TXE));
    USART3->DR = c;
}

void UART3_SendStringRaw(const char *str)
{
    while (*str)
    {
        UART3_SendCharRaw(*str++);
    }
}

HAL_StatusTypeDef UART3_SendCharWithStatus(char c)
{
    return HAL_UART_Transmit(&huart3, (uint8_t *)&c, 1, 50);
}

int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 100);
    return ch;
}
