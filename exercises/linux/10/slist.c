#include <stdio.h>
#include <stdlib.h>
#include "common.h"

/* 创建节点 */
Node *create_node(DataType data)
{
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    node->data = data;
    node->next = NULL;

    return node;
}

/* 尾插 */
int push_back(Node **head, DataType data)
{
    Node *new_node = create_node(data);

    /* 空链表 */
    if (*head == NULL)
    {
        *head = new_node;
        return -1;
    }

    Node *p = *head;

    while (p->next != NULL)
    {
        p = p->next;
    }

    p->next = new_node;

    return 0;
}

/* 查询（按值） */
Node *find(Node *head, DataType data)
{
    while (head != NULL)
    {
        if (strcmp(head->data.ip, data.ip) == 0 && head->data.port == data.port)
            return head;

        head = head->next;
    }

    return NULL;
}

/* 删除（按值删除第一个） */
int remove_node(Node **head, DataType data)
{
    if (*head == NULL)
        return 0;

    Node *cur = *head;
    Node *pre = NULL;

    while (cur != NULL)
    {
        if (strcmp(cur->data.ip, data.ip) == 0 && cur->data.port == data.port)
        {
            /* 删除头节点 */
            if (pre == NULL)
            {
                *head = cur->next;
            }
            else
            {
                pre->next = cur->next;
            }

            free(cur);
            return 1;
        }

        pre = cur;
        cur = cur->next;
    }

    return 0;
}

/* 释放 */
void destroy_list(Node **head)
{
    Node *p = *head;

    while (p != NULL)
    {
        Node *tmp = p;
        p = p->next;
        free(tmp);
    }

    *head = NULL;
}