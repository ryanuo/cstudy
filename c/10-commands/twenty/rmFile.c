#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

void rmFile(const char *path)
{
    struct stat st;

    if (lstat(path, &st) == -1)
    {
        perror(path);
        return;
    }

    // 普通文件 / 软链接：直接删
    if (!S_ISDIR(st.st_mode))
    {
        if (remove(path) == -1)
            perror(path);
        return;
    }

    // 目录：递归处理
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        perror(path);
        return;
    }

    struct dirent *entry;
    char fullpath[PATH_MAX];

    while ((entry = readdir(dir)) != NULL)
    {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(fullpath,
                 sizeof(fullpath),
                 "%s/%s",
                 path,
                 entry->d_name);

        rmFile(fullpath);
    }

    closedir(dir);

    // 删除空目录
    if (rmdir(path) == -1)
        perror(path);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file/dir>\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        rmFile(argv[i]);
    }

    return 0;
}