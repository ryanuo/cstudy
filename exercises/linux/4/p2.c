#include "p.h"

int main(int argc, char **argv)
{
    pid_t pid = 0;
    
    int fd = open(KEY_PATH_P1P2, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read = read(fd, &pid, sizeof(pid));
    if (bytes_read != sizeof(pid)) {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    printf("成功读取到目标进程 PID: %d\n", pid);
    close(fd);

    union sigval val = {.sival_int = 5};
    
    if (sigqueue(pid, SIGUSR1, val) == -1) {
        perror("sigqueue 发送失败");
        exit(EXIT_FAILURE);
    }

    printf("信号 SIGUSR1 及数据已成功发送至 PID %d\n", pid);
    return 0;
}