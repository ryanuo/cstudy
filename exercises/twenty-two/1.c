#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main()
{
    int fd[2];

    pipe(fd);

    // 获取原有标志
    int flags = fcntl(fd[1], F_GETFL);

    // 增加非阻塞属性
    fcntl(fd[1], F_SETFL, flags | O_NONBLOCK);

    char buf[1024];
    memset(buf, 'A', sizeof(buf));

    int total = 0;

    while (1)
    {
        int n = write(fd[1], buf, sizeof(buf));

        if (n == -1)
        {
            printf("\n");
            perror("write");
            printf("errno=%d\n", errno);
            printf("pipe capacity=%d bytes\n", total);
            break;
        }

        total += n;

        printf("\r已写入:%d bytes", total);
        fflush(stdout);
    }

    return 0;
}