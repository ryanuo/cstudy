#ifndef __COMMON_H__
#define __COMMON_H__

#include <time.h>

typedef enum
{
    TASK_LOW = 1,
    TASK_MID,
    TASK_HIGH
} priority_t;

typedef enum
{
    TASK_COMPLETING,
    TASK_COMPLETED
} complete_t;

typedef struct
{
    char task_id[32];
    char task_name[64];

    priority_t task_priority;

    time_t task_end_time;
    time_t task_create_time;
    complete_t completed;

} task_data_t;

typedef struct taskNode
{
    task_data_t data;
    struct taskNode *next;
} task_t;

// 操作类型枚举
typedef enum
{
    OP_ADD,    // 添加任务
    OP_REMOVE, // 删除任务
    OP_MODIFY, // 修改任务
} operation_type_t;

// 栈节点结构
typedef struct stack_node
{
    operation_type_t op_type; // 操作类型
    task_data_t task_data;    // 保存的任务数据
    char prev_task_id[32];
    struct stack_node *next;
} stack_node_t;

// 栈结构
typedef struct
{
    stack_node_t *top;
    int size;
} stackc_t;

#endif