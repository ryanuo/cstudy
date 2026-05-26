
#ifndef __DLIST_H__
#define __DLIST_H__

#include "bus.h"

typedef struct LineNode
{
    list_data_t data;

    struct LineNode *next;
    struct LineNode *prev;

} dlist_t;

int dlist_addhead(dlist_t **head, list_data_t data);

int dlist_showall(const dlist_t *head);

#endif