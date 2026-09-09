#include "dclist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/***
@function:  创建单向链表
@argment:
          head: 用于接收头指针的地址
          data: 节点数据
@retval:  成功:  0
          失败:  -1
@author:  class2604
***/

int dclist_create(dclist_t **head, dclist_data_t data)
{
    dclist_t *pnew = (dclist_t *)malloc(sizeof(dclist_t));
    if (pnew == NULL)
        return -1;
    pnew->data = data;
    pnew->prev = pnew->next = pnew;

    *head = pnew;
    return 0;
}

static void __dclist_insert(dclist_t *prev, dclist_t *pnew, dclist_t *next)
{
    pnew->next = next;
    pnew->prev = prev;
    prev->next = pnew;
    next->prev = pnew;
}

int dclist_insert(dclist_t **head, dclist_data_t data)
{
    dclist_t *pnew = (dclist_t *)malloc(sizeof(dclist_t));
    if (pnew == NULL)
        return -1;
    pnew->data = data;
    pnew->prev = pnew->next = pnew;

    if (*head == NULL)
    {
        *head = pnew;
        return 0;
    }
    __dclist_insert((*head)->prev, pnew, *head);

    return 0;
}

static void __dclist_delete(dclist_t *prev, dclist_t *pnew, dclist_t *next)
{
    prev->next = next;
    next->prev = prev;
    free(pnew);
}

int dclist_delete(dclist_t **head, dclist_data_t data)
{
    dclist_t *p = dclist_query(*head, data);
    if (p == NULL)
        return -1;

    if (p == *head)
    {
        if (p->next == p)
            *head = NULL;
        else
            *head = p->next;
    }
    __dclist_delete(p->prev, p, p->next);

    return 0;
}

dclist_t *dclist_query(const dclist_t *head, dclist_data_t data)
{
    const dclist_t *p = head;
    while (p)
    {
        if (memcmp(&p->data, &data, sizeof(dclist_data_t)) == 0)
            return (dclist_t *)p;
        p = p->next;
        if (p == head)
            break;
    }
    return NULL;
}
int dclist_update(const dclist_t *head, dclist_data_t old, dclist_data_t new)
{
    dclist_t *p = dclist_query(head, old);
    if (p == NULL)
        return -1;
    p->data = new;
    return 0;
}

int dclist_destroy(dclist_t **head)
{
    dclist_t *p = *head, *q = NULL;
    while (p)
    {
        q = p;
        p = p->next;
        free(q);

        if (p == *head)
            break;
    }
    *head = NULL;
    return 0;
}
/***
@function: 遍历单向链表数据节点
@argment:
          head: 用于接收头指针
@retval:  0
@author:  class2604
***/

int dclist_showall(const dclist_t *head)
{
    const dclist_t *p = head;
    while (p)
    {
        printf("%d ", p->data);
        p = p->next;

        if (p == head)
            break;
    }
    printf("\n");
    return 0;
}
