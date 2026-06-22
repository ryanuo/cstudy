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

    pid_t pid = fork();

    if (pid == 0)
    {
        execl("./lib/worker",
              "worker",
              argv[1],
              NULL);

        perror("execl");
    }
    else
    {
        wait(NULL);
    }

    return 0;
}