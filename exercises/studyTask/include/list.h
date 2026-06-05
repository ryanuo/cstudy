#ifndef LIST_H
#define LIST_H

#include "typing.h"

task_t *list_create_node(const task_data_t *data);

int list_insert_head(task_t **head, const task_data_t *data);

int list_insert_tail(task_t **head, const task_data_t *data);

task_t *list_find_id(task_t *head, const char *id);
task_t *list_find_name(task_t *head, const char *name);
task_t *list_find_cpd(task_t *head, complete_t, void (*callback)(const task_t *));

int list_delete(task_t **head, const char *id);

int list_insert_after(task_t **head,
                      const char *prev_id,
                      const task_data_t *data);

void list_destroy(task_t **head);

int list_size(task_t *head);

#endif