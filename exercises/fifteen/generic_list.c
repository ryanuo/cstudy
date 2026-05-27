#include <stdlib.h>
#include <string.h>
#include "generic_list.h"

/* 创建单个节点 */
static generic_list_t *_create_node(void *data, size_t data_size)
{
    generic_list_t *node = (generic_list_t *)malloc(sizeof(generic_list_t));
    if (node == NULL)
        return NULL;

    node->data = malloc(data_size);
    if (node->data == NULL)
    {
        free(node);
        return NULL;
    }

    memcpy(node->data, data, data_size);
    node->next = NULL;
    node->prev = NULL;

    return node;
}

/* ============ 单向链表操作 ============ */

int slist_create(generic_list_t **head, void *data, size_t data_size)
{
    if (head == NULL || data == NULL)
        return -1;

    generic_list_t *node = _create_node(data, data_size);
    if (node == NULL)
        return -1;

    *head = node;
    return 0;
}

int slist_add_head(generic_list_t **head, void *data, size_t data_size)
{
    if (head == NULL || data == NULL)
        return -1;

    generic_list_t *node = _create_node(data, data_size);
    if (node == NULL)
        return -1;

    node->next = *head;
    *head = node;

    return 0;
}

int slist_delete(generic_list_t **head, void *target, size_t data_size, compare_func cmp)
{
    if (head == NULL || *head == NULL || target == NULL || cmp == NULL)
        return -1;

    generic_list_t *p = *head;
    int cmp_result;

    /* 删除头节点 */
    cmp(p->data, target, &cmp_result);
    if (cmp_result == 0)
    {
        *head = p->next;
        free(p->data);
        free(p);
        return 0;
    }

    /* 删除中间或尾节点 */
    while (p->next != NULL)
    {
        cmp(p->next->data, target, &cmp_result);
        if (cmp_result == 0)
        {
            generic_list_t *temp = p->next;
            p->next = p->next->next;
            free(temp->data);
            free(temp);
            return 0;
        }
        p = p->next;
    }

    return -1;
}

int slist_print(const generic_list_t *head, print_func print)
{
    if (print == NULL)
        return -1;

    const generic_list_t *p = head;
    while (p != NULL)
    {
        print(p->data);
        p = p->next;
    }

    return 0;
}

int slist_find(generic_list_t *head, void *target, size_t data_size, compare_func cmp, void **result)
{
    if (head == NULL || target == NULL || cmp == NULL || result == NULL)
        return -1;

    generic_list_t *p = head;
    int cmp_result;

    while (p != NULL)
    {
        cmp(p->data, target, &cmp_result);
        if (cmp_result == 0)
        {
            *result = p->data;
            return 0;
        }
        p = p->next;
    }

    return -1;
}

void slist_destroy(generic_list_t **head, free_func destroy)
{
    if (head == NULL)
        return;

    generic_list_t *p = *head;
    while (p != NULL)
    {
        generic_list_t *temp = p;
        p = p->next;

        if (destroy != NULL)
            destroy(temp->data);
        else
            free(temp->data);

        free(temp);
    }

    *head = NULL;
}

/* ============ 双向链表操作 ============ */

int dlist_add_head(generic_list_t **head, void *data, size_t data_size)
{
    if (head == NULL || data == NULL)
        return -1;

    generic_list_t *node = _create_node(data, data_size);
    if (node == NULL)
        return -1;

    node->next = *head;
    node->prev = NULL;

    if (*head != NULL)
    {
        (*head)->prev = node;
    }

    *head = node;

    return 0;
}

int dlist_print(const generic_list_t *head, print_func print)
{
    if (print == NULL)
        return -1;

    const generic_list_t *p = head;
    while (p != NULL)
    {
        print(p->data);
        p = p->next;
    }

    return 0;
}

void dlist_destroy(generic_list_t **head, free_func destroy)
{
    if (head == NULL)
        return;

    generic_list_t *p = *head;
    while (p != NULL)
    {
        generic_list_t *temp = p;
        p = p->next;

        if (destroy != NULL)
            destroy(temp->data);
        else
            free(temp->data);

        free(temp);
    }

    *head = NULL;
}
