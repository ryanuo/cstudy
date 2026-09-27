#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LED.h"
#include "BEEP.h"

#define LED_TASK_PRIO 4
#define LED_STK_SIZE 128
#define BEEP_TASK_PRIO 3
#define BEEP_STK_SIZE 128

TaskHandle_t LED_handler = NULL;
TaskHandle_t BEEP_handler = NULL;

void LED_task(void *pvParameters);
void BEEP_task(void *pvParameters);

int main(void)
{
    /* 建议加上硬件初始化：时钟、NVIC 优先级分组等，比如 */
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    xTaskCreate(LED_task, "LED_task", LED_STK_SIZE, NULL, LED_TASK_PRIO, &LED_handler);
    xTaskCreate(BEEP_task, "BEEP_task", BEEP_STK_SIZE, NULL, BEEP_TASK_PRIO, &BEEP_handler);

    vTaskStartScheduler(); /* 启动调度器后不会返回 */

    while (1)
        ; /* 万一调度器启动失败，停在这 */
}

void LED_task(void *pvParameters)
{
    LED_init();
    while (1)
    {
        LED1_on();
        vTaskDelay(pdMS_TO_TICKS(500));
        LED1_off();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void BEEP_task(void *pvParameters)
{
    BEEP_init();
    while (1)
    {
        BEEP_on();
        vTaskDelay(pdMS_TO_TICKS(200));
        BEEP_off();
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}