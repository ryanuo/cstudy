#include <stdio.h>
#include <pthread.h>

#define THREAD_NUM 4

int g_no = 0;
pthread_cond_t cond;
pthread_mutex_t mutex;

void *pthread_worker(void *arg)
{
    int start_no = *(int *)arg;

    pthread_mutex_lock(&mutex);

    while (g_no != start_no)
    {
        pthread_cond_wait(&cond, &mutex);
    }

    int a[4] = {1, 2, 3, 4};
    int size = sizeof(a) / sizeof(a[0]);
    int current_no = g_no;

    for (int i = 0; i < size + 1; i++)
    {
        for (int j = 0; j < size; j++)
        {
            printf("%d", a[current_no++ % size]);
        }
    }

    printf("\n");
    g_no++;

    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    return NULL;
};

int main(int argc, char **argv)
{
    int nos[THREAD_NUM] = {0, 1, 2, 3};
    pthread_t threads[THREAD_NUM];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_create(&threads[i], NULL, pthread_worker, &nos[i]);
    }

    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}