#include <stdio.h>
#include <pthread.h>

#define THREAD_NUM 4
#define PRINT_COUNT 5

int g_no = 0;

pthread_mutex_t mutex;
pthread_cond_t cond;

void *worker(void *arg)
{
    int id = *(int *)arg;

    int a[] = {1, 2, 3, 4};
    int size = sizeof(a) / sizeof(a[0]);

    for (int i = 0; i < PRINT_COUNT; i++)
    {
        pthread_mutex_lock(&mutex);

        while (g_no % size != id)
        {
            pthread_cond_wait(&cond, &mutex);
        }

        printf("%d ", a[g_no % size]);

        g_no++;

        pthread_cond_broadcast(&cond);

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main()
{
    pthread_t tid[THREAD_NUM];
    int id[THREAD_NUM];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    for (int i = 0; i < THREAD_NUM; i++)
    {
        id[i] = i;
        pthread_create(&tid[i], NULL, worker, &id[i]);
    }

    for (int i = 0; i < THREAD_NUM; i++)
        pthread_join(tid[i], NULL);

    printf("\n");

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}