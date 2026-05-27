#ifndef __BUS_SERVICE_H__
#define __BUS_SERVICE_H__

#include "generic_list.h"

/*
 * 业务服务层
 * 处理所有 bus 相关的业务逻辑
 * 协调链表、I/O、和具体的业务规则
 */

typedef struct
{
    generic_list_t *dlist;  /* 双向链表 */
    generic_list_t *slist;  /* 单向链表 */
} bus_system_t;

/* 初始化系统 */
bus_system_t *bus_system_init(void);

/* 销毁系统 */
void bus_system_destroy(bus_system_t *system);

/* 添加线路到双向链表 */
int bus_add_to_dlist(bus_system_t *system, bus_t *bus_line);

/* 保存系统数据 */
int bus_system_save(bus_system_t *system, const char *filename);

/* 加载并处理数据 */
int bus_system_load_and_process(bus_system_t *system, const char *filename);

/* 查询最小里程线路 */
bus_t *bus_find_min_mileage(generic_list_t *head);

/* 打印线路信息 */
void bus_print_info(const void *data);

/* 比较两个bus是否相同 */
void bus_compare_func(const void *a, const void *b, int *result);

/* 显示所有线路 */
int bus_show_all(generic_list_t *head);

#endif
