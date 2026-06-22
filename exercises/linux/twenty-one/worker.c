#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: %s [ls|rm]\n", argv[0]);
        return -1;
    }

    if (strcmp(argv[1], "ls") == 0)
    {
        printf("执行 ls 操作\n");

        execlp("ls", "ls", "-l", NULL);

        perror("execlp");
    }
    else if (strcmp(argv[1], "rm") == 0)
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