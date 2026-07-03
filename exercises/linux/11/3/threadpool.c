#include "threadpool.h"

int threadpool_init(threadpool_t *pool, int tnum, int qsize)
{
    if (pool == NULL)
        return -1;

    pool -> isRunning = true;  //线程池创建初始化，就设置为运行状态
    pool->thread_num = tnum;
    pool->queue_num = 0;
    pool->queue_max_size = qsize;

    pool->head = pool->tail = NULL;

    if (pthread_mutex_init(&pool->mutex, NULL))
        return -1;
    if (pthread_cond_init(&pool->queue_empty, NULL))
    {
        pthread_mutex_destroy(&pool->mutex);
        return -1;
    }
    if (pthread_cond_init(&pool->queue_full, NULL))
    {
        pthread_mutex_destroy(&pool->mutex);
        pthread_cond_destroy(&pool->queue_empty);
        return -1;
    }
    pool->id = (pthread_t *)calloc(sizeof(pthread_t), tnum);
    if (pool->id == NULL)
    {
        pthread_mutex_destroy(&pool->mutex);
        pthread_cond_destroy(&pool->queue_empty);
        pthread_cond_destroy(&pool->queue_full);
        return -1;
    }
    for (register int i = 0; i < tnum; i++)
    {
        pthread_create(pool->id + i, NULL, thread_fun, pool);
    }

    return 0;
}

void clear(void* argp)
{
   threadpool_t *pool = (threadpool_t *)argp; 
   if(pool)
      pthread_mutex_unlock(&pool ->mutex) ;
}

void *thread_fun(void *argp)
{
    threadpool_t *pool = (threadpool_t *)argp;
    pthread_cleanup_push(clear,pool);
    for (;;)
    {
        pthread_mutex_lock(&pool->mutex);
        // 如果任务队列为空,则利用条件变量阻塞线程,等待任务到来
        while ((pool->queue_num == 0) && (pool -> isRunning ==true))
        {
            pthread_cond_wait(&pool->queue_empty, &pool->mutex);
        }
        if(pool -> isRunning ==false)
        {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        task_t *p = pool->head; // 获取队头任务
        pool->queue_num--;      // 更新任务数-1

        if (pool->queue_num == 0)
            pool->head = pool->tail = NULL;
        else
            pool->head = p->next;

        // 如果之前队列是满队的，现在有空位，通知被阻塞的任务添加函数
        if (pool->queue_num == pool->queue_max_size - 1)
            pthread_cond_signal(&pool->queue_full);

        pthread_mutex_unlock(&pool->mutex);

        //若任务正在执行,不响应取消请求,确保任务正常完成
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL); 
        (p->cb)(p->argp); // 执行任务函数
        free(p);          // 释放任务节点
        p = NULL;
        //任务执行完成，再响应取消
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    }
    pthread_cleanup_pop(0);
    return NULL;
}
int threadpool_addtask(threadpool_t *pool, void *(cb)(void *), void *argp)
{
    if (pool == NULL || cb == NULL)
        return -1;

    pthread_mutex_lock(&pool->mutex);
    /*如果任务队列满容量,则利用条件变量阻塞线程,等待条件通知*/
    while (pool->queue_num == pool->queue_max_size)
    {
        pthread_cond_wait(&pool->queue_full, &pool->mutex);
    }
    task_t *p = (task_t *)malloc(sizeof(task_t));
    if (p == NULL)
    {
        pthread_mutex_unlock(&pool->mutex);
        return -1;
    }
    /*填充任务节点数据*/
    p->cb = cb;
    p->argp = argp;
    p->next = NULL;

    // 将任务节点添加到队尾
    if (pool->tail == NULL)
    {
        pool->head = pool->tail = p;   //队列为空，队头队尾指向唯一节点
        pthread_cond_broadcast(& pool->queue_empty);  //广播通知被条件变量阻塞的线程，
    }
    else
    {
        pool->tail->next = p;
        pool->tail = p;
    }
    pool ->queue_num ++;
    pthread_mutex_unlock(&pool->mutex);

    return 0;
}

int  threadpool_destroy(threadpool_t* pool)
{
    if(pool == NULL)
        return -1;
    pool -> isRunning = false;
    pthread_cond_broadcast(&pool ->queue_empty); //考虑线程如果执行完所有任务，处于休眠状态

    pthread_mutex_destroy(&pool ->mutex);
    pthread_cond_destroy(&pool ->queue_empty);
    pthread_cond_destroy(&pool ->queue_full);

    for(register int i = 0; i < pool ->thread_num; i++)
    {
        pthread_cancel(pool->id[i]);
        pthread_join(pool->id[i],NULL);
    }
    free(pool ->id);  //回收存储线程id的堆空间
    pool -> id = NULL;

    task_t* p = pool ->head, *q = NULL;
    while(p)
    {
        q  = p;
        p  = p -> next;
        free(q);
    }
    pool ->head = pool ->tail = NULL;
    return 0;
}
