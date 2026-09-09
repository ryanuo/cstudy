#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char cmd[32] = {0};

    read(STDIN_FILENO,
         cmd,
         sizeof(cmd));

    printf("receive: %s\n", cmd);

    if (strcmp(cmd, "ls") == 0)
    {
        printf("执行 ls 操作\n");

        execlp("ls", "ls", "-l", NULL);

        perror("execlp");
    }
    else if (strcmp(cmd, "rm") == 0)
    {
        printf("删除 a.out\n");

        execlp("rm", "rm", "-f", "a.out", NULL);

        perror("execlp");
    }
    else
    {
        printf("未知命令\n");
    }

    return 0;
}