#include "uart_cmd.h"
#include "servo.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define USARTx USART1
#define USARTx_CLK_ENABLE() __HAL_RCC_USART1_CLK_ENABLE()
#define USARTx_CLK_DISABLE() __HAL_RCC_USART1_CLK_DISABLE()
#define USARTx_GPIO_PORT GPIOA
#define USARTx_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_PIN GPIO_PIN_9
#define USARTx_RX_PIN GPIO_PIN_10
#define USARTx_BAUDRATE 115200

#define RX_BUFSIZE 32

static UART_HandleTypeDef huart1;
static uint8_t rx_byte;
static uint8_t rx_buf[RX_BUFSIZE];
static uint8_t rx_idx = 0;
static uint8_t target_mode = 0;      /* Qt 设置的目标模式 */
static volatile uint8_t rx_flag = 0; /* 改为 volatile，中断与主循环共享 */

uint8_t UART_CMD_GetTargetMode(void)
{
    return target_mode;
}

void UART_CMD_SetMode(uint8_t m)
{
    target_mode = m;
}

void UART_CMD_ClearTargetMode(void)
{
    target_mode = 0;
}

void UART_CMD_Init(void)
{
    /* 使能时钟 */
    USARTx_CLK_ENABLE();
    USARTx_GPIO_CLK_ENABLE();

    /* 先将 TX 引脚拉高再配置为 AF_PP，防止 UART 初始化前 CH340 检测到虚假起始位产生 0x00 */
    {
        GPIO_InitTypeDef gpio = {0};
        gpio.Pin = USARTx_TX_PIN;
        gpio.Mode = GPIO_MODE_OUTPUT_PP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(USARTx_GPIO_PORT, &gpio);
        HAL_GPIO_WritePin(USARTx_GPIO_PORT, USARTx_TX_PIN, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    /* 配置 GPIO：PA9=TX 复用推挽输出，PA10=RX 浮空输入 */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = USARTx_TX_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(USARTx_GPIO_PORT, &gpio);

    gpio.Pin = USARTx_RX_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(USARTx_GPIO_PORT, &gpio);

    /* 配置 USART1 */
    huart1.Instance = USARTx;
    huart1.Init.BaudRate = USARTx_BAUDRATE;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);

    /* 手动启用 USART1 NVIC 中断（CubeMX 未配置） */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);

    /* 等待 TX 线稳定，避免首字节垃圾 */
    HAL_Delay(10);

    /* 启动上报一次状态 */
    {
        char buf[64];
        int len = sprintf(buf, "READY MODE:10 LED:OFF FAN:OFF\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, 100);
    }
}

void UART_CMD_Process(void)
{
    if (!rx_flag)
        return;

    /* 调试：回传接收到的命令 */
    {
        char dbg[64];
        int len = sprintf(dbg, "[RX]%s\n", rx_buf);
        HAL_UART_Transmit(&huart1, (uint8_t *)dbg, len, 100);
    }

    if (strncmp((char *)rx_buf, "SERVO:", 6) == 0) {
        /* 舵机角度命令，格式: SERVO:90 */
        int angle = atoi((char *)rx_buf + 6);
        if (angle >= 0 && angle <= 180) {
            servo_angle = (uint8_t)angle;
            Servo_SetAngle(servo_angle);
        }
    }
    else if (strncmp((char *)rx_buf, "MODE:", 5) == 0) {
        /* 模式切换命令，格式: MODE:3 */
        int m = atoi((char *)rx_buf + 5);
        if (m >= 1 && m <= 11)
        {
            target_mode = (uint8_t)m;
            /* 回传新状态 */
            char buf[64];
            uint8_t led = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);
            uint8_t fan = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
            int len = sprintf(buf, "MODE:%d LED:%s FAN:%s\n",
                              target_mode,
                              led == 0 ? "ON" : "OFF",
                              fan == 1 ? "ON" : "OFF");
            HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, 100);
        }
    }
    else if (strstr((char *)rx_buf, "STATUS") != NULL)
    {
        /* 回传当前状态 */
        char buf[64];
        uint8_t led = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);
        uint8_t fan = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
        int len = sprintf(buf, "MODE:%d LED:%s FAN:%s\n",
                          target_mode,
                          led == 0 ? "ON" : "OFF",
                          fan == 1 ? "ON" : "OFF");
        HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, 100);
    }

    /* 重置缓冲区 */
    rx_idx = 0;
    rx_flag = 0;
    memset(rx_buf, 0, RX_BUFSIZE);
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1); /* 重新启动接收 */
}

/* USART1 中断回调：字节到达 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USARTx)
        return;

    if (rx_byte == '\n' || rx_byte == '\r' || rx_idx >= RX_BUFSIZE - 1)
    {
        rx_buf[rx_idx] = '\0';
        rx_flag = 1; /* 通知主循环解析 */
    }
    else
    {
        rx_buf[rx_idx++] = rx_byte;
    }
    /* 重新启动接收（在中断里再次调用不会嵌套） */
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

/* USART1 中断向量（CubeMX 命名） */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}
