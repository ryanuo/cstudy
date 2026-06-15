#include <stdio.h>
#include <pthread.h>

#define THREAD_NUM 4
#define MAX_NUM 1000000

typedef struct
{
    long start;
    long end;
    long long sum;
} ThreadData;

void *worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;

    data->sum = 0;

    for (long i = data->start; i <= data->end; i++)
    {
        data->sum += i;
    }

    return NULL;
}

int main(void)
{
    pthread_t tid[THREAD_NUM];
    ThreadData data[THREAD_NUM];

    long step = MAX_NUM / THREAD_NUM;

    for (int i = 0; i < THREAD_NUM; i++)
    {
        data[i].start = i * step + 1;

        if (i == THREAD_NUM - 1)
            data[i].end = MAX_NUM;
        else
            data[i].end = (i + 1) * step;

        pthread_create(&tid[i], NULL, worker, &data[i]);
    }

    long long total = 0;

    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(tid[i], NULL);
        total += data[i].sum;
    }

    printf("sum = %lld\n", total);

    return 0;
}