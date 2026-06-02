#ifndef __TASK_H_
#define __TASK_H_

#include "common.h"

void task_add(task_t **head);
int task_remove(int task_id);
int task_update(int task_id);
void task_search(task_t *head);
int task_complete(int task_id);
int task_list_completed(void);
int task_summary(void);

#endif