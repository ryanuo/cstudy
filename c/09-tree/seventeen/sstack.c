#include <stdio.h>
#include <stdlib.h>
#include "sstack.h"

int sstack_create(sstack_t *ss, int sz)
{
    ss->pData = (sstack_data_t *)calloc(sizeof(sstack_data_t), sz);

    if (ss->pData == NULL)
        return -1;

    ss->size = sz;
    ss->top = -1;

    return 0;
}

int sstack_isfull(sstack_t *p)
{
    return p->top == p->size - 1;
}

int sstack_push(sstack_t *s, sstack_data_t data)
{
    if (sstack_isfull(s))
        return -1;
    s->pData[++s->top] = data;
    return 0;
}

int sstack_isempty(sstack_t *p)
{
    return p->top == -1;
}

int sstack_pop(sstack_t *s, sstack_data_t *data)
{
    if (sstack_isfull(s))
        return -1;
    *data = s->pData[s->top--];
    return 0;
}

void sstack_destory(sstack_t *s)
{
    if (s->pData)
    {
        free(s->pData);
        s->pData = NULL;
    }
    s->size = 0;
    s->top = -1;
}