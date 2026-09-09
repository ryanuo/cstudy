#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

int main()
{
    int fd[2];
    pid_t pid;

    if (pipe(fd) < 0)
    {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        close(fd[0]);

        int fileFd = open("test.txt", O_RDONLY);
        if (fileFd < 0)
        {
            perror("open");
            exit(1);
        }

        char buf[1024];
        ssize_t n = read(fileFd, buf, sizeof(buf) - 1);
        if (n < 0)
        {
            perror("read");
            close(fileFd);
            exit(1);
        }
        buf[n] = '\0';

        write(fd[1], buf, n);

        close(fileFd);
        close(fd[1]);
        exit(0);
    }
    else
    {
        close(fd[1]);

        char buf[1024];
        ssize_t n = read(fd[0], buf, sizeof(buf) - 1);
        if (n < 0)
        {
            perror("read");
            close(fd[0]);
            exit(1);
        }
        buf[n] = '\0';

        printf("子进程传递的数据是：%s\n", buf);

        close(fd[0]);
        wait(NULL);
    }

    return 0;
}