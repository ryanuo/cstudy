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

    fd_fifo = open(FIFO_PATH, O_RDONLY);

    if (fd_fifo < 0)
    {
        perror("open fifo");
        return -1;
    }

    fd_file = open(
        "target.txt",
        O_WRONLY | O_CREAT | O_TRUNC,
        0644
    );

    if (fd_file < 0)
    {
        perror("open target");
        return -1;
    }

    while ((n = read(fd_fifo, buf, sizeof(buf))) > 0)
    {
        write(fd_file, buf, n);
    }

    close(fd_file);
    close(fd_fifo);

    printf("receive done\n");

    return 0;
}