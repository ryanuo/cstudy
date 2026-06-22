#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>

typedef struct
{
    int file_count;
    int dir_count;
} INFO;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <directory>\n", argv[0]);
        return -1;
    }

    int fd[2];

    if (pipe(fd) < 0)
    {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    else if (pid == 0)
    {
        close(fd[0]);

        INFO info = {0};

        DIR *dir = opendir(argv[1]);

        if (dir == NULL)
        {
            perror("opendir");
            exit(-1);
        }

        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL)
        {
            if (!strcmp(entry->d_name, ".") ||
                !strcmp(entry->d_name, ".."))
            {
                continue;
            }

            if (entry->d_type == DT_DIR)
            {
                info.dir_count++;
            }
            else if (entry->d_type == DT_REG)
            {
                info.file_count++;
            }
        }

        write(fd[1], &info, sizeof(info));

        closedir(dir);
        close(fd[1]);

        exit(0);
    }

    else
    {
        close(fd[1]);

        INFO info;

        read(fd[0], &info, sizeof(info));

        wait(NULL);

        printf("普通文件数量: %d\n", info.file_count);
        printf("子目录数量: %d\n", info.dir_count);

        close(fd[0]);
    }

    return 0;
}