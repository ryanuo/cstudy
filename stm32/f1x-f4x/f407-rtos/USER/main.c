#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LED.h"
#include "BEEP.h"
#include "DELAY.h"   /* 开机标记用它的 DWT 忙等延时 */

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
    uint8_t i;

    /* FreeRTOS 要求 4 bit 全部作为抢占优先级（复位默认已等价，显式写更保险） */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* 外设初始化放在调度器启动之前，开机标记才有灯可点 */
    LED_init();
    BEEP_init();

    /* 开机标记：新固件在跑 -> LED1(PF9, 板子丝印 LED0) 快闪 3 次。
       没有这个动作就说明新固件没编进去/没下载，别往下查逻辑。 */
    for (i = 0; i < 3; i++)
    {
        LED1_on();
        DELAY_ms(80);
        LED1_off();
        DELAY_ms(80);
    }

    /* 任务创建失败不要静默：LED2(PF10, 板子丝印 LED1) 慢闪报警（多半是 heap 不够） */
    if ((xTaskCreate(LED_task, "LED_task", LED_STK_SIZE, NULL, LED_TASK_PRIO, &LED_handler) != pdPASS) ||
        (xTaskCreate(BEEP_task, "BEEP_task", BEEP_STK_SIZE, NULL, BEEP_TASK_PRIO, &BEEP_handler) != pdPASS))
    {
        while (1)
        {
            LED2_on();
            DELAY_ms(300);
            LED2_off();
            DELAY_ms(700);
        }
    }

    vTaskStartScheduler(); /* 启动调度器后不会返回 */

    while (1)
    {
        LED2_on(); /* 走到这里 = 调度器启动失败，通常是 heap 不够 */
    }
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