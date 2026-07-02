#ifndef __SLIST_H
#define __SLIST_H

#include "header.h"

typedef  sin_t   slist_data_t;
typedef struct _slist
{
    slist_data_t        data;
    struct _slist       *next;
}slist_t;

int slist_create(slist_t**,slist_data_t);
int slist_addhead(slist_t**,slist_data_t);
int slist_addtail(slist_t**,slist_data_t);
int slist_insert(slist_t**,slist_data_t,slist_data_t);


int slist_delete(slist_t**,slist_data_t);

slist_t* slist_query(const slist_t*,slist_data_t);
int slist_update(const slist_t*,slist_data_t,slist_data_t);

int slist_destroy(slist_t**);
int slist_showall(const slist_t*);

#endif
