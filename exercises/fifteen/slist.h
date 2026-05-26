
#ifndef __SLIST_H__
#define __SLIST_H__

#include "bus.h"

typedef struct SListNode
{
    list_data_t data;
    struct SListNode *next;

} slist_t;

int slist_addhead(slist_t **head, list_data_t data);
int slist_delete(slist_t **head, list_data_t target);
#endif