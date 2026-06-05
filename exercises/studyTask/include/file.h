#ifndef __FILE_IO_H__
#define __FILE_IO_H__

#include "typing.h"

#define FILE_NAME "data/task.csv"

void file_init(void);

int task_load(task_t **task);

int task_save(task_t *head);

#endif