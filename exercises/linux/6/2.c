#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define WINDOW_NUM 4

static int tickets = 100;

pthread_mutex_t mutex;

void *sell_ticket(void *arg)
{
    int id = *(int *)arg;

    while (1)
    {
        pthread_mutex_lock(&mutex);

        if (tickets <= 0)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        printf(
            "窗口%d 卖出第 %d 张票，剩余 %d 张\n",
            id,
            tickets,
            tickets - 1
        );

        tickets--;

        pthread_mutex_unlock(&mutex);

        usleep(100000);
    }

    printf("窗口%d 下班\n", id);

    return NULL;
}

int main()
{
    pthread_t tids[WINDOW_NUM];
    int ids[WINDOW_NUM];

    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < WINDOW_NUM; i++)
    {
        ids[i] = i + 1;

        pthread_create(
            &tids[i],
            NULL,
            sell_ticket,
            &ids[i]
        );
    }

    for (int i = 0; i < WINDOW_NUM; i++)
    {
        pthread_join(tids[i], NULL);
    }

    printf("\n100张票已全部售完\n");

    pthread_mutex_destroy(&mutex);

    return 0;
}