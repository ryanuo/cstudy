#include <stdlib.h>
#include <stdio.h>

#include "header.h"

#define FILE_NAME "userinfo.csv"

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

static int read_userinfo(FILE *fp, user_info_t *data)
{
    long end_time;
    long create_time;
    int priority;
    int completed;

    if (fscanf(fp,
               " %31[^,],%31s",
               data->username,
               data->password) != 2)
    {
        return -1;
    }

    return 0;
}

int userinfo_load(user_list_t **head)
{
    FILE *fp = fopen(FILE_NAME, "r");

    if (fp == NULL)
    {
        return -1;
    }

    user_info_t data;

    while (read_userinfo(fp, &data) == 0)
    {
        if (push_back(head, data) != 0)
        {
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);

    return 0;
}

int userinfo_save(user_list_t *head)
{
    FILE *fp = fopen(FILE_NAME, "w");
    if (!fp)
    {
        perror("fopen failed");
        return -1;
    }

    for (user_list_t *cur = head; cur != NULL; cur = cur->next)
    {
        fprintf(fp,
                "%s,%s\n",
                cur->data.username,
                cur->data.password);
    }

    fclose(fp);
    return 0;
}