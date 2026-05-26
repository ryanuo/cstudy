#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dlist.h"

int dlist_insert(dlist_t **head, dlist_data_t target, dlist_data_t data)
{
    dlist_t *pnew = (dlist_t *)malloc(sizeof(dlist_t));

    if (pnew == NULL)
        return -1;

    pnew->data = data;
    pnew->next = pnew->prev = NULL;

    if (*head == NULL || memcmp(&(*head)->data, &target, sizeof(dlist_data_t)) == 0)
    {
        pnew->next = *head;
        if (*head != = NULL)
            (*head)->prev = pnew;

        *head = pnew;
        return 0;
    }

    dlist_t *p = *head;
    while (p->next != NULL && memcmp(&p->next->data, &target, sizeof(dlist_data_t)) != 0)
    {
        p = p->next;
    }

    if (p->next == NULL)
    {
        free(pnew);
        return -1;
    }

    pnew->next = p->next;
    p->next->prev = pnew;

    pnew->prev = p;
    p->next = pnew;

    return 0;
}

int dlist_delete(dlist_t **head, dlist_data_t target)
{
    if (head == NULL || *head == NULL)
    {
        return -1;
    }

    if ((*head)->data == target)
    {
        dlist_t *temp = *head;
        *head = (*head)->next;
        if (*head != NULL)
        {
            (*head)->prev = NULL;
        }

        free(temp);
        return 0;
    }

    dlist_t *p = *head;

    while (p->next != NULL && p->next->data != target)
    {
        p = p->next;
    }

    if (p->next == NULL)
    {
        return -1;
    }

    dlist_t *temp = p->next;
    p->next = temp->next;
    if (temp->next != NULL)
    {
        temp->next->prev = p;
    }

    free(temp);

    return 0;
}