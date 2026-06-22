#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define THREAD_NUM 5

void *worker(void *arg)
{
    int id = *(int *)arg;

    for (int i = 1; i <= 10; i++)
    {
        printf("Thread %d: message %d\n", id, i);
        usleep(100000);
    }

    return NULL;
}

int main(void)
{
    pthread_t tid[THREAD_NUM];
    int ids[THREAD_NUM];

    for (int i = 0; i < THREAD_NUM; i++)
    {
        ids[i] = i + 1;
        pthread_create(&tid[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(tid[i], NULL);
    }

    return 0;
}
