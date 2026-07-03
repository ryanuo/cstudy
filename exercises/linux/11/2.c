#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>  // 1. 补全头文件
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void *worker(void *arg)
{
    pthread_mutex_lock(&m);
    sleep(2);
    pthread_mutex_unlock(&m);
    return NULL;
}

int main()
{
    pthread_t t;
    pthread_create(&t, NULL, worker, NULL);

    sleep(1);

    if (fork() == 0)
    {
        sleep(10);
        printf("时间到\n");
        _exit(0);  // 2. 子进程做完事立刻退出，绝不碰父进程的锁和线程！
    }

    wait(NULL);    // 3. 父进程安心等待子进程结束

    printf("父进程时间到\n");
    pthread_join(t, NULL);
    
    return 0;
}