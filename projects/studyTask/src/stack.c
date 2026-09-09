#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "stack.h"

// 创建栈
stackc_t *stack_create()
{
    stackc_t *stack = (stackc_t *)malloc(sizeof(stackc_t));
    if (!stack)
        return NULL;

    stack->top = NULL;
    stack->size = 0;
    return stack;
}

// 入栈 - 保存操作
void stack_push(stackc_t *stack,
                operation_type_t op_type,
                const task_data_t *data,
                const char *prev_task_id)
{
    if (!stack)
        return;

    stack_node_t *new_node = (stack_node_t *)malloc(sizeof(stack_node_t));
    if (!new_node)
        return;

    new_node->op_type = op_type;
    if (data)
    {
        memcpy(&new_node->task_data, data, sizeof(task_data_t));
    }

    if (prev_task_id)
    {
        strcpy(new_node->prev_task_id,
               prev_task_id);
    }

    new_node->next = stack->top;
    stack->top = new_node;
    stack->size++;

    printf("✓ 操作已保存到撤销栈\n");
}

// 出栈 - 获取上一次操作
int stack_pop(stackc_t *stack,
              operation_type_t *op_type,
              task_data_t *data,
              char *prev_task_id)
{
    if (!stack || !stack->top)
    {
        return -1;
    }

    stack_node_t *node = stack->top;
    *op_type = node->op_type;
    if (data)
    {
        memcpy(data, &node->task_data, sizeof(task_data_t));
    }

    if (prev_task_id)
    {
        strcpy(prev_task_id,
               node->prev_task_id);
    }

    stack->top = node->next;
    stack->size--;

    free(node);
    return 0;
}

// 检查栈是否为空
int stack_is_empty(stackc_t *stack)
{
    return !stack || !stack->top;
}

// 销毁栈
void stack_destroy(stackc_t *stack)
{
    if (!stack)
        return;

    while (stack->top)
    {
        stack_node_t *temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
    free(stack);
}

// 撤销操作
int undo_previous_operation(task_t **head, stackc_t *stack)
{
    if (stack_is_empty(stack))
    {
        printf("✗ 没有可撤销的操作！\n");
        return -1;
    }

    operation_type_t op_type;
    task_data_t data;
    char prev_task_id[32] = {0};

    if (stack_pop(stack, &op_type, &data, prev_task_id) != 0)
    {
        return -1;
    }

    switch (op_type)
    {
    case OP_ADD:
        list_delete(head, data.task_id);
        printf("✓ 已撤销添加操作\n");
        break;

    case OP_REMOVE:
        list_insert_after(head, prev_task_id, &data);
        printf("✓ 已撤销删除操作\n");
        break;

    case OP_MODIFY:
    {
        task_t *cur = *head;
        while (cur)
        {
            if (strcmp(cur->data.task_id, data.task_id) == 0)
            {
                memcpy(&cur->data, &data, sizeof(task_data_t));
                printf("✓ 已撤销修改操作\n");
                return 0;
            }
            cur = cur->next;
        }
        printf("✗ 任务不存在\n");
        return -1;
    }
    }

    return 0;
}