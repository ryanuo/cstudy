#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }
    else if (pid == 0)
    {
        printf("this is child process\n");
    }
    else
    {
        printf("this is parent process\n");
    }

    return 0;
}