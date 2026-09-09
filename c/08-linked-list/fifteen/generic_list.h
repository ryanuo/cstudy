#ifndef __GENERIC_LIST_H__
#define __GENERIC_LIST_H__

#include <stddef.h>

/*
 * 通用链表接口
 * 支持任意数据类型
 * 使用函数指针实现自定义比较、输出、释放操作
 */

typedef void (*compare_func)(const void *a, const void *b, int *result);
typedef void (*print_func)(const void *data);
typedef void (*free_func)(void *data);

typedef struct GenericNode
{
    void *data;
    struct GenericNode *next;
    struct GenericNode *prev;
} generic_list_t;

/* 单向链表操作 */
int slist_create(generic_list_t **head, void *data, size_t data_size);
int slist_add_head(generic_list_t **head, void *data, size_t data_size);
int slist_delete(generic_list_t **head, void *target, size_t data_size, compare_func cmp);
int slist_print(const generic_list_t *head, print_func print);
int slist_find(generic_list_t *head, void *target, size_t data_size, compare_func cmp, void **result);
void slist_destroy(generic_list_t **head, free_func destroy);

/* 双向链表操作 */
int dlist_add_head(generic_list_t **head, void *data, size_t data_size);
int dlist_print(const generic_list_t *head, print_func print);
void dlist_destroy(generic_list_t **head, free_func destroy);

#endif
