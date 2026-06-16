#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();

    if(pid == 0)
    {
        exit(100);
    }

    int status;

    wait(&status);

    if(WIFEXITED(status))
    {
        printf("child exit code = %d\n",
               WEXITSTATUS(status));
    }

    return 0;
}