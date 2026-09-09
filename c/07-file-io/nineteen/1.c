#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    int c = access("./1.c", X_OK);
    printf("%d\n", c);

    if (c == -1)
    {
        perror("Error access");
    }

    int fd;
    fd = open("./2.c", O_CREAT, S_IWUSR | S_IRUSR);
    close(fd);
    return 0;
}