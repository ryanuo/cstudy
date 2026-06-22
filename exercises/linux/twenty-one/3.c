#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: %s [ls|rm]\n", argv[0]);
        return -1;
    }

    int fd[2];

    pipe(fd);

    pid_t pid = fork();

    if (pid == 0)
    {
        close(fd[1]);

        dup2(fd[0], STDIN_FILENO);
        execl("./lib/worker1",
              "worker1",
              NULL);

        perror("execl");
    }
    else
    {
        close(fd[0]);

        write(fd[1], argv[1], 2);

        close(fd[1]);

        wait(NULL);
    }

    return 0;
}