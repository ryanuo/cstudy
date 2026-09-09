#ifndef __STACK_H_
#define __STACK_H_

#include "typing.h"

// 栈操作函数
stackc_t *stack_create();
void stack_push(stackc_t *stack,
                operation_type_t op_type,
                const task_data_t *data,
                const char *prev_task_id);

int stack_pop(stackc_t *stack,
              operation_type_t *op_type,
              task_data_t *data,
              char *prev_task_id);

int stack_is_empty(stackc_t *stack);
void stack_destroy(stackc_t *stack);
int undo_previous_operation(task_t **head, stackc_t *stack);

#endif