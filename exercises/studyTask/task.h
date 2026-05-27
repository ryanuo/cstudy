#ifndef __TASK_H_
#define __TASK_H_
#include <time.h>

typedef enum
{
    TASK_LOW,
    TASK_MID,
    TASK_HIGH
} priority_t;

typedef struct
{
    char task_id[32];
    char task_name[64];

    priority_t task_priority;

    time_t task_end_time;
    time_t task_create_time;

} task_t;

int task_create();
int task_delete();
int task_modify(char *task_name);
int task_query(char *val, priority_t);

#endif