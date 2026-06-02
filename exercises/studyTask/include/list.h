#ifndef LIST_H
#define LIST_H

#include "common.h"

task_t *list_create_node(const task_data_t *data);

int list_insert_head(task_t **head, const task_data_t *data);

int list_insert_tail(task_t **head, const task_data_t *data);

task_t *list_find(task_t *head, const char *id);

int list_delete(task_t **head, const char *id);

void list_destroy(task_t **head);

int list_size(task_t *head);

#endif