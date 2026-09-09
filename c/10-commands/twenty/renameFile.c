#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

void to_upper(char *str)
{
    while (*str)
    {
        if (*str >= 'a' && *str <= 'z')
        {
            *str -= ('a' - 'A');
        }
        str++;
    }
}

void renameFile(const char *path)
{
    struct stat st;

    if (lstat(path, &st) == -1)
    {
        perror(path);
        return;
    }

    if (!S_ISDIR(st.st_mode))
    {
        char newpath[PATH_MAX];

        snprintf(newpath,
                 sizeof(newpath),
                 "%s",
                 path);

        to_upper(newpath);

        if (strcmp(path, newpath) != 0)
        {
            if (rename(path, newpath) == -1)
            {
                perror("rename");
            }
        }

        return;
    }

    DIR *dir = opendir(path);

    if (dir == NULL)
    {
        perror(path);
        return;
    }

    struct dirent *entry;
    char child[PATH_MAX];

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(child,
                 sizeof(child),
                 "%s/%s",
                 path,
                 entry->d_name);

        renameFile(child);
    }

    closedir(dir);

    char newpath[PATH_MAX];

    snprintf(newpath,
             sizeof(newpath),
             "%s",
             path);

    to_upper(newpath);

    if (strcmp(path, newpath) != 0)
    {
        if (rename(path, newpath) == -1)
        {
            perror("rename");
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr,
                "Usage: %s <file_or_dir>\n",
                argv[0]);
        return 1;
    }

    renameFile(argv[1]);

    return 0;
}