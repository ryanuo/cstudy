#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_PATH "/Users/ryanuo/tmp/myfifo"

int main(void)
{
    int fd_fifo;
    int fd_file;

    char buf[1024];

    ssize_t n;

    fd_fifo = open(FIFO_PATH, O_WRONLY);

    if (fd_fifo < 0)
    {
        perror("open fifo");
        return -1;
    }

    fd_file = open("source.txt", O_RDONLY);

    if (fd_file < 0)
    {
        perror("open source");
        return -1;
    }

    while ((n = read(fd_file, buf, sizeof(buf))) > 0)
    {
        write(fd_fifo, buf, n);
    }

    close(fd_file);
    close(fd_fifo);

    printf("send done\n");

    return 0;
}