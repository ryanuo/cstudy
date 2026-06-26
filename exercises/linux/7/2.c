#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_THREADS 3

enum
{
    EMPTY = 0,
    APPLE = 1,
    BANANA
};

int plate = 0;

sem_t empty;
sem_t full;

pthread_mutex_t mutex;

void *thread_producer(void *arg)
{
    int fruits[2] = {APPLE, BANANA};
    for (int i = 0; i < 2; i++)
    {
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        plate = fruits[i];
        printf("妈妈放入的水果为：%s\n", fruits[i] == APPLE ? "苹果" : "香蕉");
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
        sleep(1);
    }

    return NULL;
}

void *thread_consumer(void *arg)
{
    int role = *(int *)arg;

    int my_fruit;

    sem_wait(&full);
    pthread_mutex_lock(&mutex);
    my_fruit = plate;
    plate = EMPTY;
    pthread_mutex_unlock(&mutex);

    printf("消费者 %s: 取出水果 %s\n",
           role == 1 ? "儿子" : "女儿",
           my_fruit == APPLE ? "苹果" : "香蕉");

    sem_post(&empty);
    sleep(1);

    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t threads[MAX_THREADS];
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, 1);
    sem_init(&full, 0, 0);

    int consumers[2] = {1, 2};

    pthread_create(&threads[0], NULL, thread_producer, NULL);
    pthread_create(&threads[1], NULL, thread_consumer, &consumers[0]);
    pthread_create(&threads[2], NULL, thread_consumer, &consumers[1]);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);

    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}