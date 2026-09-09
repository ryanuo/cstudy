#ifndef __STACK__H
#define __STACK__H

typedef int sstack_data_t;
typedef struct
{
    sstack_data_t *pData;
    int top;
    int size;
} sstack_t;

int sstack_create(sstack_t *ss, int sz);
int sstack_isfull(sstack_t *p);
int sstack_push(sstack_t *s, sstack_data_t data);
int sstack_isempty(sstack_t *p);
int sstack_pop(sstack_t *s, sstack_data_t *data);
void sstack_destory(sstack_t *s);

#endif