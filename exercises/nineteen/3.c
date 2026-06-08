#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int fd;

    if (argc > 1)
        for (int i = 1; i < argc; i++)
        {
            fd = access(argv[i], F_OK);
            if (fd == -1)
                fprintf(stderr, "文件 %s 不存在\n", argv[i]);
        }
    else
        fprintf(stderr, "命令行未输入文件、目录\n");

    return 0;
}