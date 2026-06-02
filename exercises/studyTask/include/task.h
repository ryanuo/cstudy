#ifndef __TASK_H_
#define __TASK_H_

#include "common.h"

void task_add(task_t **head);
int task_remove(int task_id);
int task_update(int task_id);
void task_search(task_t *head);
void task_complete_mod(task_t **head);
void task_list_completed(task_t *head);
void task_summary(task_t *head);

void query_all_task(task_t *head);

#endif