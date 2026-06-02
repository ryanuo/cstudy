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

int task_add(void);
int task_remove(int task_id);
int task_update(int task_id);
int task_search(const char *keyword,
                priority_t priority);
int task_complete(int task_id);
int task_list_completed(void);
int task_summary(void);

#endif