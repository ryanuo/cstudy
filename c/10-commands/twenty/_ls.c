#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>

char get_file_type(mode_t mode)
{
    if (S_ISREG(mode))
        return '-';

    if (S_ISDIR(mode))
        return 'd';

    if (S_ISLNK(mode))
        return 'l';

    if (S_ISCHR(mode))
        return 'c';

    if (S_ISBLK(mode))
        return 'b';

    if (S_ISFIFO(mode))
        return 'p';

    if (S_ISSOCK(mode))
        return 's';

    return '?';
}

void print_permission(mode_t mode)
{
    const mode_t perms[] =
        {
            S_IRUSR, S_IWUSR, S_IXUSR,
            S_IRGRP, S_IWGRP, S_IXGRP,
            S_IROTH, S_IWOTH, S_IXOTH};

    const char chars[] = "rwxrwxrwx";

    for (int i = 0; i < 9; i++)
    {
        putchar(mode & perms[i] ? chars[i] : '-');
    }
}

void print_mtime(time_t mtime)
{
    char buf[64];

    strftime(
        buf,
        sizeof(buf),
        "%b %d %H:%M",
        localtime(&mtime));

    printf("%s", buf);
}

void print_file_info(const char *path)
{
    struct stat st;

    if (lstat(path, &st) == -1)
    {
        perror(path);
        return;
    }

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *grp = getgrgid(st.st_gid);

    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;

    printf("%c", get_file_type(st.st_mode));

    print_permission(st.st_mode);

    printf(" %2lu",
           (unsigned long)st.st_nlink);

    printf(" %-8s",
           pw ? pw->pw_name : "unknown");

    printf(" %-8s",
           grp ? grp->gr_name : "unknown");

    printf(" %8lld",
           (long long)st.st_size);

    printf(" ");

    print_mtime(st.st_mtime);

    printf(" %s\n", name);
}

void list_directory(const char *dir_path)
{
    DIR *dir = opendir(dir_path);

    if (dir == NULL)
    {
        perror(dir_path);
        return;
    }

    struct dirent *entry;
    char fullpath[PATH_MAX];

    while ((entry = readdir(dir)) != NULL)
    {
        /* 跳过隐藏文件 */
        if (entry->d_name[0] == '.')
            continue;

        snprintf(
            fullpath,
            sizeof(fullpath),
            "%s/%s",
            dir_path,
            entry->d_name);

        print_file_info(fullpath);
    }

    closedir(dir);
}

void process_paths(int argc, char *argv[])
{
    struct stat st;

    if (argc == 1)
    {
        list_directory(".");
        return;
    }

    for (int i = 1; i < argc; i++)
    {
        if (lstat(argv[i], &st) == -1)
        {
            perror(argv[i]);
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            if (argc > 2)
                printf("\n%s:\n", argv[i]);

            list_directory(argv[i]);
        }
        else
        {
            print_file_info(argv[i]);
        }
    }
}

int main(int argc, char *argv[])
{
    process_paths(argc, argv);
    return 0;
}