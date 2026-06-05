#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

/**
 * 创建节点
 */
task_t *list_create_node(const task_data_t *data)
{
    task_t *node = malloc(sizeof(task_t));
    if (node == NULL)
    {
        return NULL;
    }

    node->data = *data;
    node->next = NULL;

    return node;
}

/**
 * 头插
 */
int list_insert_head(task_t **head, const task_data_t *data)
{
    task_t *node = list_create_node(data);
    if (node == NULL)
    {
        return -1;
    }

    node->next = *head;
    *head = node;

    return 0;
}

/**
 * 尾插
 */
int list_insert_tail(task_t **head, const task_data_t *data)
{
    task_t *node = list_create_node(data);
    if (node == NULL)
    {
        return -1;
    }

    if (*head == NULL)
    {
        *head = node;
        return 0;
    }

    task_t *cur = *head;

    while (cur->next)
    {
        cur = cur->next;
    }

    cur->next = node;

    return 0;
}

/**
 * 根据ID查找
 */
task_t *list_find_id(task_t *head, const char *id)
{
    while (head)
    {
        if (strcmp(head->data.task_id, id) == 0)
        {
            return head;
        }

        head = head->next;
    }

    return NULL;
}

/**
 * 根据名称查找
 */
task_t *list_find_name(task_t *head, const char *name)
{
    while (head)
    {
        if (strcmp(head->data.task_name, name) == 0)
        {
            return head;
        }

        head = head->next;
    }

    return NULL;
}

/**
 * 根据完成情况查找
 */
task_t *list_find_cpd(task_t *head, complete_t cpd)
{
    while (head)
    {
        if (head->data.completed == cpd)
        {
            printf("%s\n", head->data.task_name);
        }

        head = head->next;
    }

    return NULL;
}

/**
 * 根据ID删除
 */
int list_delete(task_t **head, const char *id)
{
    if (head == NULL || *head == NULL)
    {
        return -1;
    }

    /* 删除头节点 */
    if (strcmp((*head)->data.task_id, id) == 0)
    {
        task_t *tmp = *head;

        *head = tmp->next;

        free(tmp);

        return 0;
    }

    task_t *cur = *head;

    while (cur->next &&
           strcmp(cur->next->data.task_id, id) != 0)
    {
        cur = cur->next;
    }

    if (cur->next == NULL)
    {
        return -1;
    }

    task_t *tmp = cur->next;

    cur->next = tmp->next;

    free(tmp);

    return 0;
}

/**
 * 销毁链表
 */
void list_destroy(task_t **head)
{
    if (head == NULL)
    {
        return;
    }

    task_t *cur = *head;

    while (cur)
    {
        task_t *next = cur->next;

        free(cur);

        cur = next;
    }

    *head = NULL;
}

/**
 * 获取链表长度
 */
int list_size(task_t *head)
{
    int count = 0;

    while (head)
    {
        count++;
        head = head->next;
    }

    return count;
}

int list_insert_after(task_t **head,
                      const char *prev_id,
                      const task_data_t *data)
{
    if (!head || !data)
    {
        return -1;
    }

    task_t *node =
        (task_t *)malloc(sizeof(task_t));

    if (!node)
    {
        return -1;
    }

    memcpy(&node->data,
           data,
           sizeof(task_data_t));

    node->next = NULL;

    // 删除的是头节点
    if (strlen(prev_id) == 0)
    {
        node->next = *head;
        *head = node;
        return 0;
    }

    task_t *cur = *head;

    while (cur)
    {
        if (strcmp(cur->data.task_id,
                   prev_id) == 0)
        {
            node->next = cur->next;
            cur->next = node;
            return 0;
        }

        cur = cur->next;
    }

    free(node);

    return -1;
}