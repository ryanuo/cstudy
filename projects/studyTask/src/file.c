#include <stdio.h>
#include <stdlib.h>

#include "file.h"
#include "list.h"
#include "utils.h"

void file_init(void)
{
    FILE *fp = fopen(FILE_NAME, "r");

    if (fp == NULL)
    {
        fp = fopen(FILE_NAME, "w");

        if (fp != NULL)
        {
            fclose(fp);
        }
    }
    else
    {
        fclose(fp);
    }
}

static int read_task(FILE *fp, task_data_t *data)
{
    long end_time;
    long create_time;
    int priority;
    int completed;

    if (fscanf(fp,
               "%31[^,],%63[^,],%d,%ld,%ld,%d\n",
               data->task_id,
               data->task_name,
               &priority,
               &end_time,
               &create_time,
               &completed) != 6)
    {
        return -1;
    }

    data->task_priority = (priority_t)priority;
    data->task_end_time = (time_t)end_time;
    data->task_create_time = (time_t)create_time;
    data->completed = (complete_t)completed;

    return 0;
}

int task_load(task_t **head)
{
    FILE *fp = fopen(FILE_NAME, "r");

    if (fp == NULL)
    {
        return -1;
    }

    task_data_t data;

    while (read_task(fp, &data) == 0)
    {
        if (list_insert_tail(head, &data) != 0)
        {
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);

    return 0;
}

int task_save(task_t *head)
{
    FILE *fp = fopen(FILE_NAME, "w");
    if (!fp)
    {
        perror("fopen failed");
        return -1;
    }

    for (task_t *cur = head; cur != NULL; cur = cur->next)
    {
        // 🔥 1. 插入前打印（关键）
        // printf("DEBUG NODE:\n");
        // printf("id   = [%s]\n", cur->data.task_id);
        // printf("name = [%s]\n", cur->data.task_name);
        // printf("prio = %d\n", cur->data.task_priority);
        // printf("end  = %ld\n", (long)cur->data.task_end_time);
        // printf("crt  = %ld\n", (long)cur->data.task_create_time);
        // printf("done = %d\n", cur->data.completed);

        fprintf(fp,
                "%s,%s,%d,%ld,%ld,%d\n",
                cur->data.task_id,
                cur->data.task_name,
                (int)cur->data.task_priority,
                (long)cur->data.task_end_time,
                (long)cur->data.task_create_time,
                (int)cur->data.completed);
    }

    fclose(fp);
    return 0;
}