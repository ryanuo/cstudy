#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

void _head(char *filename, int line_num)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return;
    }

    char buf[1024] = {0};
    ssize_t n = 0;
    int line_count = 0;

    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0)
    {
        for (ssize_t i = 0; i < n; i++)
        {
            write(STDOUT_FILENO, &buf[i], 1);

            if (buf[i] == '\n')
            {
                line_count++;
                if (line_count == line_num)
                {
                    goto close_file;
                }
            }
        }
    }

close_file:
    close(fd);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "用法: %s <文件名> [行数]\n", argv[0]);
        return 1;
    }

    int lines = 10;

    if (argc >= 3)
    {
        lines = (int)strtol(argv[2], NULL, 10);
        if (lines <= 0)
        {
            fprintf(stderr, "错误: 行数必须是一个正整数\n");
            return 1;
        }
    }

    _head(argv[1], lines);

    return 0;
}