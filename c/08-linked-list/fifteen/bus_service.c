#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bus_service.h"
#include "bus_io.h"
#include "generic_list.h"

/* ===== 比较和输出函数 ===== */

void bus_compare_func(const void *a, const void *b, int *result)
{
    if (a == NULL || b == NULL || result == NULL)
    {
        if (result)
            *result = -1;
        return;
    }

    bus_t *bus_a = (bus_t *)a;
    bus_t *bus_b = (bus_t *)b;

    int cmp_line = strcmp(bus_a->line_name, bus_b->line_name);
    int cmp_start = strcmp(bus_a->start_stat, bus_b->start_stat);
    int cmp_end = strcmp(bus_a->end_stat, bus_b->end_stat);
    float mileage_diff = bus_a->mileage - bus_b->mileage;

    if (cmp_line == 0 && cmp_start == 0 && cmp_end == 0 && mileage_diff == 0.0f)
    {
        *result = 0;  /* 相同 */
    }
    else
    {
        *result = 1;  /* 不同 */
    }
}

void bus_print_info(const void *data)
{
    if (data == NULL)
        return;

    bus_t *bus = (bus_t *)data;
    printf("%s,%s,%s,%.2f\n",
           bus->line_name,
           bus->start_stat,
           bus->end_stat,
           bus->mileage);
}

/* ===== 业务逻辑 ===== */

bus_system_t *bus_system_init(void)
{
    bus_system_t *system = (bus_system_t *)malloc(sizeof(bus_system_t));
    if (system == NULL)
        return NULL;

    system->dlist = NULL;
    system->slist = NULL;

    return system;
}

void bus_system_destroy(bus_system_t *system)
{
    if (system == NULL)
        return;

    slist_destroy(&system->dlist, NULL);
    slist_destroy(&system->slist, NULL);

    free(system);
}

int bus_add_to_dlist(bus_system_t *system, bus_t *bus_line)
{
    if (system == NULL || bus_line == NULL)
        return -1;

    return dlist_add_head(&system->dlist, bus_line, sizeof(bus_t));
}

int bus_system_save(bus_system_t *system, const char *filename)
{
    if (system == NULL || filename == NULL)
        return -1;

    return bus_save_to_file(filename, system->dlist);
}

bus_t *bus_find_min_mileage(generic_list_t *head)
{
    if (head == NULL)
        return NULL;

    bus_t *min_bus = (bus_t *)head->data;
    generic_list_t *p = head->next;

    while (p != NULL)
    {
        bus_t *current = (bus_t *)p->data;
        if (current->mileage < min_bus->mileage)
        {
            min_bus = current;
        }
        p = p->next;
    }

    return min_bus;
}

int bus_system_load_and_process(bus_system_t *system, const char *filename)
{
    if (system == NULL || filename == NULL)
        return -1;

    /* 加载数据到单向链表 */
    int ret = bus_load_from_file(filename, &system->slist);
    if (ret != 0)
        return ret;

    /* 查找最小里程的线路 */
    bus_t *min_bus = bus_find_min_mileage(system->slist);

    if (min_bus != NULL)
    {
        printf("最小里程线路: %s\n", min_bus->line_name);

        /* 从单向链表中删除 */
        slist_delete(&system->slist, min_bus, sizeof(bus_t), bus_compare_func);
    }

    return 0;
}

int bus_show_all(generic_list_t *head)
{
    return slist_print(head, bus_print_info);
}
