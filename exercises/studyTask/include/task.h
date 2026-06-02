#ifndef __TASK_H_
#define __TASK_H_

#include "common.h"

void task_add(task_t **head);
int task_remove(int task_id);
void task_search(task_t *head);
/**
 * 任务修改
 * opt：
 * 1. 任务名称
 * 2. 优先级(1-3)
 * 3. 完成状态(0未完成/1已完成)
 * 4. 结束时间（格式：YYYY-MM—DD hh-mm-ss）
 */
int task_update(task_t *head, const char *id, int opt);
void task_complete_mod(task_t *head);
void task_list_completed(task_t *head);
void task_summary(task_t *head);

void query_all_task(task_t *head);

void task_mod(task_t *head);

#endif