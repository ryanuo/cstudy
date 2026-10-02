#include "bsp_esp8266.h"
#include <string.h>

/* ================= 内部全局变量 ================= */

static UART_HandleTypeDef *esp_huart = NULL;

/* 环形缓冲：中断里写，主循环里读 */
static uint8_t  esp_rx_buf[ESP8266_RX_BUF_SIZE];
static volatile uint16_t esp_rx_head = 0;
static volatile uint16_t esp_rx_tail = 0;

/* 累积文本缓冲：主循环里做字符串查找 */
static char     esp_acc[ESP8266_ACC_SIZE];
static uint16_t esp_acc_len = 0;

/* 中断里单字节接收（必须非 static，让 main.c 也能访问） */
uint8_t esp_rx_byte;

/* ================= 内部函数 ================= */

/* 把环形缓冲里的新字节搬到累积缓冲 */
static void esp_pump(void)
{
    uint16_t head = esp_rx_head;
    uint16_t tail = esp_rx_tail;

    if (head == tail) return;

    while (tail != head) {
        if (esp_acc_len < ESP8266_ACC_SIZE - 1) {
            esp_acc[esp_acc_len++] = (char)esp_rx_buf[tail];
        }
        tail = (tail + 1) % ESP8266_RX_BUF_SIZE;
    }
    esp_acc[esp_acc_len] = '\0';
    esp_rx_tail = head;
}

/* ================= 对外接口 ================= */

void BSP_ESP8266_Init(UART_HandleTypeDef *huart)
{
    esp_huart = huart;

    esp_rx_head = 0;
    esp_rx_tail = 0;
    esp_acc_len = 0;
    esp_acc[0]  = '\0';

    /* 启动第一次单字节中断接收，后续在回调里自动续接 */
    HAL_UART_Receive_IT(esp_huart, &esp_rx_byte, 1);
}

void BSP_ESP8266_SendAT(char *cmd)
{
    while (*cmd) {
        HAL_UART_Transmit(esp_huart, (uint8_t *)cmd++, 1, HAL_MAX_DELAY);
    }
    HAL_UART_Transmit(esp_huart, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
}

void BSP_ESP8266_SendData(uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(esp_huart, data, len, HAL_MAX_DELAY);
}

void BSP_ESP8266_RxCallback(uint8_t data)
{
    uint16_t next_head = (esp_rx_head + 1) % ESP8266_RX_BUF_SIZE;

    /* 缓冲满就丢掉这个字节，避免覆盖未读数据 */
    if (next_head != esp_rx_tail) {
        esp_rx_buf[esp_rx_head] = data;
        esp_rx_head = next_head;
    }

    /* 重新开启下一次中断接收 */
    if (esp_huart) {
        HAL_UART_Receive_IT(esp_huart, &esp_rx_byte, 1);
    }
}

void BSP_ESP8266_ClearBuffer(void)
{
    esp_rx_tail = esp_rx_head;
    esp_acc_len = 0;
    esp_acc[0]  = '\0';
}

uint8_t BSP_ESP8266_Contains(char *expected)
{
    esp_pump();
    return (strstr(esp_acc, expected) != NULL) ? 1 : 0;
}

char *BSP_ESP8266_Find(char *pattern)
{
    esp_pump();
    return strstr(esp_acc, pattern);
}