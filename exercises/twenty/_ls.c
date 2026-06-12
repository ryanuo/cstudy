#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

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
    char perm[] = "rwxrwxrwx";

    for (int i = 0; i < 9; i++)
        putchar(mode & (1 << (8 - i)) ? perm[i] : '-');
}

void print_mtime(time_t mtime)
{
    char buf[64];

    strftime(
        buf,
        sizeof(buf),
        "%b %d %H:%M",
        localtime(&mtime));

    printf("%s ", buf);
}

void print_file_info(const char *path)
{
    struct stat st;

    if (lstat(path, &st) == -1)
    {
        perror(path);
        return;
    }

    printf("%c", get_file_type(st.st_mode));
    print_permission(st.st_mode);

    printf(" %2lu", (unsigned long)st.st_nlink);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *grp = getgrgid(st.st_gid);

    printf(" %-8s", pw ? pw->pw_name : "unknown");
    printf(" %-8s", grp ? grp->gr_name : "unknown");

    printf(" %8lld", (long long)st.st_size);

    printf(" ");
    print_mtime(st.st_mtime);

    printf(" %s\n", path);
}

void process_paths(int argc, char *argv[])
{
    if (argc == 1)
    {
        print_file_info(".");
        return;
    }

    for (int i = 1; i < argc; i++)
    {
        print_file_info(argv[i]);
    }
}

int main(int argc, char *argv[])
{
    process_paths(argc, argv);
    return 0;
}