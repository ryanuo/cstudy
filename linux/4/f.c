#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

static pid_t child_pid;

void alarm_handler(int sig)
{
    (void)sig;

    printf("超时，杀死子进程 %d\n", child_pid);

    kill(child_pid, SIGKILL);
}

int main()
{
    child_pid = fork();

    if (child_pid < 0)
    {
        perror("fork");
        return -1;
    }

    if (child_pid == 0)
    {
        printf("child start\n");

        sleep(20);

        printf("child exit\n");

        exit(0);
    }

    signal(SIGALRM, alarm_handler);

    alarm(10);
    for (int i = 10; i > 0; i--)
    {
        printf("倒计时: %2d 秒\n", i);
        sleep(1);
    }

    // 等待子进程结束
    waitpid(child_pid, NULL, 0);

    alarm(0); // 取消定时器

    printf("parent exit\n");

    return 0;
}