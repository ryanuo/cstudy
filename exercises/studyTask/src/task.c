#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "utils.h"
#include "list.h"
#include "stack.h"

static void print_task(const task_t *res)
{
    printf("%-10s %-20s %-8d %-20s %-20s %-10s\n",
           res->data.task_id,
           res->data.task_name,
           res->data.task_priority,
           time_to_str(res->data.task_end_time),
           time_to_str(res->data.task_create_time),
           res->data.completed == TASK_COMPLETED ? "已完成" : "进行中");
}

task_data_t build_task()
{
    task_data_t data;
    char time_str[64];

    printf("请输入任务ID: ");
    scanf("%31s", data.task_id);

    printf("请输入任务名称: ");
    scanf("%63s", data.task_name);

    data.task_priority = input_int("请输入优先级(1-3): ", 1, 3);

    data.task_create_time = time(NULL);

    clear_buffer();

    printf("请输入结束时间 (2026-06-02 12:00:00 / 直接回车=0): ");
    fgets(time_str, sizeof(time_str), stdin);

    trim_newline(time_str);

    if (strlen(time_str) == 0)
    {
        data.task_end_time = 0;
    }
    else
    {
        data.task_end_time = string_to_timestamp(time_str);
    }

    data.completed = input_int("是否完成(0:未完成 1:已完成): ", 0, 1);

    return data;
}

void task_add(task_t **head, stackc_t *stack)
{
    task_data_t data;

    data = build_task();

    if (list_insert_tail(head, &data) != 0)
    {
        printf("添加失败\n");
        return;
    }

    // 保存操作到栈
    if (stack)
    {
        stack_push(stack, OP_ADD, &data, "");
    }

    printf("添加成功！\n");
}

int task_update(task_t *head, const char *id, int opt, stackc_t *stack)
{
    task_t *cur = head;
    task_data_t old_data;
    char buf[64];

    while (cur)
    {
        if (strcmp(cur->data.task_id, id) == 0)
        {
            memcpy(&old_data, &cur->data, sizeof(task_data_t));
            switch (opt)
            {
            case 1:
                printf("输入新任务名称：");
                fgets(cur->data.task_name, sizeof(cur->data.task_name), stdin);
                trim_newline(cur->data.task_name);
                break;

            case 2:
                cur->data.task_priority = input_int(
                    "请输入新优先级(1-3): ",
                    1,
                    3);
                break;

            case 3:
                cur->data.completed = (complete_t)input_int(
                    "是否完成(0:未完成 1:已完成): ",
                    0,
                    1);
                break;

            case 4:
                printf("输入新的结束时间(时间戳)：");
                fgets(buf, sizeof(buf), stdin);
                cur->data.task_end_time = (time_t)atol(buf);
                break;

            default:
                printf("无效操作\n");
                return -1;
            }

            printf("更新成功！\n");
            // 修改后保存到栈
            if (stack)
            {
                stack_push(stack, OP_MODIFY, &old_data, "");
            }
            return 0;
        }

        cur = cur->next;
    }

    printf("未找到任务\n");
    return -1;
}

void task_mod(task_t *head, stackc_t *stack)
{
    printf("请输入要修改的任务ID:\n");

    char task_id[64] = {0};
    if (!fgets(task_id, sizeof(task_id), stdin))
        return;
    trim_newline(task_id);

    int opt;
    char buf[64];

    printf("1.修改名称\n2.修改优先级\n3.修改状态\n");
    printf("请选择要修改的信息：");

    fgets(buf, sizeof(buf), stdin);
    opt = atoi(buf);

    task_update(head, task_id, opt, stack);
}

void task_remove(task_t **head, stackc_t *stack)
{
    printf("请输入要删除的任务ID:\n");

    char task_id[64] = {0};

    if (!fgets(task_id,
               sizeof(task_id),
               stdin))
    {
        return;
    }

    trim_newline(task_id);

    task_t *cur = *head;
    task_t *prev = NULL;

    task_data_t backup_data;

    char prev_task_id[32] = {0};

    while (cur)
    {
        if (strcmp(cur->data.task_id,
                   task_id) == 0)
        {
            memcpy(&backup_data,
                   &cur->data,
                   sizeof(task_data_t));

            if (prev)
            {
                strcpy(prev_task_id,
                       prev->data.task_id);
            }

            break;
        }

        prev = cur;
        cur = cur->next;
    }

    if (!cur)
    {
        printf("任务不存在\n");
        return;
    }

    if (list_delete(head, task_id) == 0)
    {
        if (stack)
        {
            stack_push(
                stack,
                OP_REMOVE,
                &backup_data,
                prev_task_id);
        }

        printf("删除成功\n");
    }
    else
    {
        printf("删除失败\n");
    }
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

void task_complete_mod(task_t *head, stackc_t *stack)
{
    printf("请输入要修改的任务ID\n");

    char task_id[64] = {0};
    if (!fgets(task_id, sizeof(task_id), stdin))
        return;

    trim_newline(task_id);

    task_update(head, task_id, 3, stack);
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

void query_all_task(task_t *head)
{
    if (head == NULL)
    {
        printf("暂无任务！\n");
        return;
    }

    printf("\n");
    printf("=============== 任务列表 ===============\n");

    printf("%-10s %-20s %-8s %-20s %-20s %-8s\n",
           "ID", "名称", "优先级", "结束时间", "创建时间", "状态");

    printf("--------------------------------------------------------------------------------------\n");

    for (task_t *cur = head; cur != NULL; cur = cur->next)
        print_task(cur);

    printf("--------------------------------------------------------------------------------------\n");
}
