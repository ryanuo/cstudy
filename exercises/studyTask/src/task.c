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

static void print_task(const task_t *res)
{
    printf("找到任务：\n");
    printf("ID: %s\n", res->data.task_id);
    printf("Name: %s\n", res->data.task_name);
    printf("Priority: %d\n", res->data.task_priority);
    printf("End: %s\n", time_to_str(res->data.task_end_time));
    printf("Create: %s\n", time_to_str(res->data.task_create_time));
    printf("Completed: %d\n", res->data.completed);
}

void task_search(task_t *head)
{
    char buf[64];
    int val;

    printf("请选择查询方式：1.ID  2.名称\n");

    if (!fgets(buf, sizeof(buf), stdin))
        return;

    val = atoi(buf);

    char search_str[64] = {0};

    if (val == 1)
    {
        printf("请输入任务ID：");
    }
    else if (val == 2)
    {
        printf("请输入任务名称：");
    }
    else
    {
        printf("输入不合法！！！\n");
        return;
    }

    if (!fgets(search_str, sizeof(search_str), stdin))
        return;

    trim_newline(search_str);

    task_t *res = NULL;

    if (val == 1)
        res = list_find_id(head, search_str);
    else
        res = list_find_name(head, search_str);

    if (res)
        print_task(res);
    else
        printf("未找到任务\n");
}

void task_list_completed(task_t *head)
{
    printf("以下任务已完成：\n");
    list_find_cpd(head, 1);

    printf("\033[32m--------------------------------------\033[0m\n");
}

void task_summary(task_t *head)
{
    int total = 0;
    int completed = 0;
    int uncompleted = 0;

    task_t *cur = head;

    while (cur)
    {
        total++;

        if (cur->data.completed == TASK_COMPLETED)
        {
            completed++;
        }
        else
        {
            uncompleted++;
        }

        cur = cur->next;
    }

    printf("\n");
    printf("========== 任务统计 ==========\n");
    printf("总任务数   : %d\n", total);
    printf("已完成数量 : %d\n", completed);
    printf("未完成数量 : %d\n", uncompleted);
    printf("==============================\n");
}