#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex;
int g_var = 0;
void cancel_thread()
{
    pthread_mutex_unlock(&mutex);
}

void *g_val_1(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);

        pthread_cleanup_push(cancel_thread, NULL);

        g_var = 1;

        usleep(100000);

        pthread_mutex_unlock(&mutex);
        pthread_cleanup_pop(0);
    }

    return NULL;
}

void *g_val_2(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        printf("thread2: %d\n", g_var);
        pthread_mutex_unlock(&mutex);

        usleep(100000);
    }

    return NULL;
}

void *g_val_3(void *arg)
{
    pthread_t *tids = arg;

    sleep(5);

    printf("\n取消线程1\n\n");

    pthread_cancel(tids[0]);

    return NULL;
}

int main()
{
    pthread_t tids[3];

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&tids[0], NULL, g_val_1, NULL);
    pthread_create(&tids[1], NULL, g_val_2, NULL);
    pthread_create(&tids[2], NULL, g_val_3, tids);

    pthread_join(tids[0], NULL);

    pthread_join(tids[2], NULL);

    sleep(3);

    printf("main结束\n");

    printf("线程2继续运行\n");
    pthread_exit(NULL);

    return 0;
}