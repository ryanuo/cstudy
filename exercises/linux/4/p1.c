#include "p.h"

volatile sig_atomic_t got_signal = 0;
volatile sig_atomic_t countdown_sec = 0;

void sig_handler(int signum, siginfo_t *info, void *context)
{
    (void)signum;
    (void)context;

    countdown_sec = info->si_value.sival_int;
    got_signal = 1;
}

int main(int argc, char **argv)
{
    if (mkfifo(KEY_PATH_P1P2, 0666) == -1 && errno != EEXIST)
    {
        perror("mkfifo");
        return EXIT_FAILURE;
    }

    pid_t pid = getpid();
    int fd = open(KEY_PATH_P1P2, O_WRONLY);
    if (fd == -1)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    if (write(fd, &pid, sizeof(pid)) == -1)
    {
        perror("write");
        close(fd);
        return EXIT_FAILURE;
    }
    close(fd);

    struct sigaction act = {0};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sig_handler;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR1, &act, NULL) == -1)
    {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    printf("进程 PID: %d 正在等待信号...\n", pid);

    while (1)
    {
        if (got_signal)
        {
            got_signal = 0;

            printf("接收到秒数为: %d，启动 alarm 定时器\n", (int)countdown_sec);

            alarm(countdown_sec);
            for (;;)
            {
                sleep(1);
                printf("倒计时: %d 秒\n", --countdown_sec);
            }
        }
    }

    return 0;
}