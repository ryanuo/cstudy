#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();

    if(pid == 0)
    {
        execlp("/bin/ls",
              "ls",
              "-l",
              NULL);
    }

    wait(NULL);

    printf("child finished\n");

    return 0;
}