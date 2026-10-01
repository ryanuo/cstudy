#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "task.h"
#include "LED.h"
#include "BEEP.h"
#include "DELAY.h"
#include "USART.h"
#include "KEY.h"
#include "OLED.h"
#include "DHT11.h"

#define LED_TASK_PRIO 1
#define LED_STK_SIZE 128
#define BEEP_TASK_PRIO 2
#define BEEP_STK_SIZE 128
#define KEY_TASK_PRIO 3
#define KEY_STK_SIZE 128
#define DHT11_TASK_PRIO 4
#define DHT11_STK_SIZE 128

TaskHandle_t LED_handler = NULL;
TaskHandle_t BEEP_handler = NULL;
TaskHandle_t KEY_handler = NULL;
TaskHandle_t DHT11_handler = NULL;

void LED_task(void *pvParameters);
void BEEP_task(void *pvParameters);
void KEY_task(void *pvParameters);
void DHT11_task(void *pvParameters);

QueueHandle_t QUEUE_handler = NULL;

typedef struct
{
    uint8_t temp;
    uint8_t hum;
} dht11_t;

int main(void)
{
    DELAY_init(168);
    USART1_init();
    /* 建议加上硬件初始化：时钟、NVIC 优先级分组等，比如 */
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    QUEUE_handler = xQueueCreate(10, sizeof(dht11_t));
    xTaskCreate(LED_task, "LED_task", LED_STK_SIZE, NULL, LED_TASK_PRIO, &LED_handler);
    xTaskCreate(BEEP_task, "BEEP_task", BEEP_STK_SIZE, NULL, BEEP_TASK_PRIO, &BEEP_handler);
    // xTaskCreate(KEY_task, "KEY_task", KEY_STK_SIZE, NULL, KEY_TASK_PRIO, &KEY_handler);
    xTaskCreate(DHT11_task, "DHT11_task", DHT11_STK_SIZE, NULL, DHT11_TASK_PRIO, &DHT11_handler);

    vTaskStartScheduler(); /* 启动调度器后不会返回 */

    while (1)
        ; /* 万一调度器启动失败，停在这 */
}

void LED_task(void *pvParameters)
{
    LED_init();
    OLED_Init();

    while (1)
    {
        USART1_Printf("led_value\r\n");
        LED1_on();
        vTaskDelay(pdMS_TO_TICKS(500));
        LED1_off();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void BEEP_task(void *pvParameters)
{
    BEEP_init();
    dht11_t dht11_data;
    char buf[32];

    while (1)
    {
        USART1_Printf("beep_value\r\n");
        LED2_on();
        vTaskDelay(pdMS_TO_TICKS(800));

        // 非阻塞拿数据，有就刷新，没有就用旧值
        if (xQueueReceive(QUEUE_handler, &dht11_data, 0) == pdTRUE)
        {
            sprintf(buf, "T:%dC H:%d%%", dht11_data.temp, dht11_data.hum);
            USART1_Printf("%s\r\n", buf);
            OLED_ShowString(0, 0, buf, OLED_8X16);
            OLED_Update();
        }
    }
}

void KEY_task(void *pvParameters)
{
    KEY_init();
    uint8_t key_value = 0;

    while (1)
    {
        key_value = KEY_getvalue();
        USART1_Printf("key_value:%d\r\n", key_value);
        if (3 == key_value)
        {
            vTaskSuspend(BEEP_handler);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // 关键：让出 CPU，10ms 扫描一次
    }
}

void DHT11_task(void *pvParameters)
{
    DHT11_Init();
    dht11_t dht11;
    uint8_t t = 0, h = 0;
    while (1)
    {
        if (DHT11_Read(&t, &h))
        {
            dht11.temp = t;
            dht11.hum = h;
            xQueueSend(QUEUE_handler, &dht11, pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}