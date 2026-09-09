/*************************************************************************
  > File Name:    threadpool.h
  > Author:       Terry Yu
  > Description:  
  > Created Time: 2026-07-03 10:46:05
 ************************************************************************/

#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include "header.h"
/*定义任务结构体，表示线程要执行的任务*/
typedef struct _task
{
    void*(*cb)(void*);             //函数指针，指向实际要执行的任务函数
    void*          argp;         //传递给任务函数的参数
    struct _task  *next;         //指向下一个任务的指针，用于构建任务队列
}task_t;

/*定义线程池结构体 */
typedef struct 
{
     bool              isRunning;       //表示线程池的运行状态
     int               thread_num;     //线程池中线程数量
     int               queue_num;      //任务队列中当前的任务数量
     int               queue_max_size;  //任务队列最大的任务数(容量)

     pthread_mutex_t   mutex;           //互斥锁
     pthread_cond_t    queue_empty;     //用于阻塞线程函数的条件变量(队列为空时)
     pthread_cond_t    queue_full;      //用于阻塞任务添加函数的条件变量(队列为满时)

     task_t             *head;          //任务队列的队头指针
     task_t             *tail;          //任务队列的队尾指针

     pthread_t          *id;            //用于开辟堆内存，存储线程Id
 
}threadpool_t;

/*线程池初始化函数 */
int threadpool_init(threadpool_t  *pool, int tnum,int qsize);
/*线程函数*/
void* thread_fun(void* argp);
/*任务添加函数*/
int  threadpool_addtask(threadpool_t* pool,void*(cb)(void*),void* argp);

/*线程池回收函数*/
int  threadpool_destroy(threadpool_t* pool);
#endif //_THREADPOOL_H