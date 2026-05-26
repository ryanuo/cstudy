#include <stdlib.h>
#include <stdio.h>

#include "bus.h"
#include "slist.h"

int slist_addhead(slist_t **head, list_data_t data)
{
    slist_t *pnew = (slist_t *)malloc(sizeof(slist_t));

    if (pnew == NULL)
        return -1;

    pnew->data = data;
    pnew->next = *head;

    *head = pnew;

    return 0;
}

int slist_delete(slist_t **head, list_data_t target)
{
    if (head == NULL || *head == NULL)
    {
        return -1;
    }

    if (compare_bus((*head)->data, target))
    {
        slist_t *temp = *head;
        *head = (*head)->next;
        free(temp);
        return 0;
    }

    slist_t *p = *head;
    while (p->next != NULL && !compare_bus(p->next->data, target))
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
