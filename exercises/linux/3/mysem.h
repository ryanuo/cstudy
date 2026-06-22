#ifndef __MYSEM_H
#define __MYSEM_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdbool.h>
#include <errno.h>

bool mysem_exist(key_t key);
int mysem_create(key_t key,int nsems);
int mysem_init(int semid,int index,int initvalue);
int mysem_p(int semid,int index);
int mysem_v(int semid,int index);
int mysem_destroy(int semid);

#endif