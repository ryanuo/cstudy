#include <stdio.h>
#include <stdlib.h>

#include "task.h"
#include "utils.h"
#include "list.h"

task_data_t build_task()
{
    task_data_t data;
    char time_str[64];
    int priority;
    int completed = 0;

    printf("请输入任务ID: ");
    scanf("%31s", data.task_id);

    printf("请输入任务名称: ");
    scanf("%63s", data.task_name);

    printf("请输入优先级(1-3): ");
    scanf("%d", &priority);

    data.task_priority = priority;

    data.task_create_time = time(NULL);

    clear_buffer();

    printf("请输入结束时间 (2026-06-02 12:00:00 / 直接回车=0): ");
    fgets(time_str, sizeof(time_str), stdin);

    data.completed = completed;

    return data;
}

void task_add(task_t **head)
{
    task_data_t data;

    data = build_task();

    if (list_insert_tail(head, &data) != 0)
    {
        printf("添加失败\n");
        return;
    }

    printf("添加成功！\n");
}