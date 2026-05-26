#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "slist.h"

int slist_create(slist_t **head, slist_data_t data)
{
    slist_t *pnew = (slist_t *)malloc(sizeof(slist_t));

    if (pnew == NULL)
        return -1;

    pnew->data = data;
    pnew->next = NULL;

    *head = pnew;

    return 0;
}

int slist_addhead(slist_t **head, slist_data_t data)
{
    slist_t *pnew = (slist_t *)malloc(sizeof(slist_t));

    if (pnew == NULL)
        return -1;

    pnew->data = data;
    pnew->next = *head;

    *head = pnew;

    return 0;
}

int slist_addtail(slist_t **head, slist_data_t data)
{
    slist_t *pnew = (slist_t *)malloc(sizeof(slist_t));

    if (pnew == NULL)
        return -1;

    pnew->data = data;
    pnew->next = NULL;

    if (*head == NULL)
    {
        *head = pnew;
    }
    else
    {
        slist_t *ptail = *head;
        while (ptail->next)
            ptail = ptail->next;

        ptail->next = pnew;
    }

    return 0;
}

int slist_showall(const slist_t *head)
{
    const slist_t *p = head;

    while (p)
    {
        printf("%d\n", p->data);
        p = p->next;
    }

    printf("\n");
    return 0;
}

int slist_insert(slist_t **head, slist_data_t target, slist_data_t data)
{
    slist_t *pnew = (slist_t *)malloc(sizeof(slist_t));

    if (pnew == NULL)
        return -1;

    pnew->data = data;
    pnew->next = NULL;

    if (*head == NULL || (*head)->data == target)
    {
        pnew->next = *head;
        *head = pnew;
        return 0;
    }

    slist_t *p = *head;
    while (p->next != NULL && p->next->data != target)
        p = p->next;

    if (p->next != NULL)
        pnew->next = p->next;

    p->next = pnew;

    return 0;
}

int slist_delete(slist_t **head, slist_data_t target)
{
    if (head == NULL || *head == NULL)
    {
        return -1;
    }

    if ((*head)->data == target)
    {
        slist_t *temp = *head;
        *head = (*head)->next;
        free(temp);
        return 0;
    }

    slist_t *p = *head;
    while (p->next != NULL && p->next->data != target)
    {
        p = p->next;
    }

    if (p->next != NULL)
    {
        slist_t *temp = p->next;
        p->next = p->next->next;
        free(temp);
        return 0;
    }

    return -1;
}

int slist_destory(slist_t **head)
{
    slist_t *p = *head, *q;

    while (p)
    {
        q = p;
        p = p->next;
        free(q);
    }
    *head = NULL;

    return 0;
}

int slist_search(slist_t *head, slist_data_t data)
{
    slist_t *p = head;

    while (p)
    {
        if (memcmp(&p->data, &data, sizeof(slist_data_t)) == 0)
        {
            return 0;
        }
        p = p->next;
    }

    return -1;
}

int slist_update(slist_t **head, slist_data_t target, slist_data_t data)
{
    slist_t *p = *head;

    while (p)
    {
        if (memcmp(&p->data, &target, sizeof(slist_data_t)) == 0)
        {
            p->data = data;
        }

        p = p->next;
    }

    return -1;
}