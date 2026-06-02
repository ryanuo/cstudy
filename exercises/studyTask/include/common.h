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

#endif