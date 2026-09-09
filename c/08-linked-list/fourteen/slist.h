#ifndef __SLIST__H
#define __SLIST__H

typedef int slist_data_t;
typedef struct ListNode
{
    slist_data_t data;
    struct ListNode *next;
} slist_t;

int slist_create(slist_t **, slist_data_t);
int slist_addhead(slist_t **, slist_data_t);
int slist_addtail(slist_t **, slist_data_t);
int slist_insert(slist_t **, slist_data_t, slist_data_t);
int slist_destory(slist_t **);
slist_t *slist_search(const slist_t *, slist_data_t);
int slist_update(slist_t **, slist_data_t, slist_data_t);

int slist_showall(const slist_t *);

#endif