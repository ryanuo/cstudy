#ifndef __BUS_H__
#define __BUS_H__


typedef struct busLine
{
    char line_name[20];
    char start_stat[50];
    char end_stat[50];
    float mileage;
} bus_t;

typedef bus_t list_data_t;
#include "slist.h"
#include "dlist.h"

typedef struct BaseNode
{
    list_data_t data;
    struct BaseNode *next;
} base_node_t;


int compare_bus(list_data_t a, list_data_t b);
int save_busdata(const base_node_t *head);

bus_t *find_min_data(slist_t *head);
int read_del_sdata(slist_t **head);
#endif