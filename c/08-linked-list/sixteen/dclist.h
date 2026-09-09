#ifndef __DCLIST_H
#define __DCLIST_H

typedef int dclist_data_t;
typedef struct _dclist
{
    dclist_data_t data;
    struct _dclist *prev;
    struct _dclist *next;
} dclist_t;

int dclist_create(dclist_t **, dclist_data_t);
int dclist_insert(dclist_t **, dclist_data_t);

int dclist_delete(dclist_t **, dclist_data_t);

dclist_t *dclist_query(const dclist_t *, dclist_data_t);
int dclist_update(const dclist_t *, dclist_data_t, dclist_data_t);

int dclist_destroy(dclist_t **);
int dclist_showall(const dclist_t *);

#endif
