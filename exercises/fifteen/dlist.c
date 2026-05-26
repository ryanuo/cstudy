#include <stdlib.h>
#include <stdio.h>

#include "bus.h"
#include "dlist.h"

int dlist_addhead(dlist_t **head, list_data_t data)
{
    dlist_t *pnew = (dlist_t *)malloc(sizeof(dlist_t));

    if (pnew == NULL)
        return -1;

    pnew->data = data;
    pnew->next = *head;
    pnew->prev = NULL;

    if (*head != NULL)
    {
        (*head)->prev = pnew;
    }

    *head = pnew;

    return 0;
}

int dlist_showall(const dlist_t *head)
{
    const dlist_t *p = head;

    while (p)
    {
        printf("%s,%s,%s,%.2f\n",
               p->data.line_name,
               p->data.start_stat,
               p->data.end_stat,
               p->data.mileage);

        p = p->next;
    }

    printf("\n");
    return 0;
}
