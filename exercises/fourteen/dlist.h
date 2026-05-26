#ifndef __DLIST__H
#define __DLIST__H

typedef int dlist_data_t;
typedef struct ListNode
{
    dlist_data_t data;
    struct ListNode *next;
    struct ListNode *prev;
} dlist_t;

int dlist_create(dlist_t **, dlist_data_t);
int dlist_addhead(dlist_t **, dlist_data_t);
int dlist_addtail(dlist_t **, dlist_data_t);
int dlist_insert(dlist_t **, dlist_data_t, dlist_data_t);
int dlist_destory(dlist_t **);
dlist_t *dlist_search(const dlist_t *, dlist_data_t);
int dlist_update(dlist_t **, dlist_data_t, dlist_data_t);

int dlist_showall(const dlist_t *);

#endif