#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main()
{
    int shmid = shmget(
        IPC_PRIVATE,
        4096,
        IPC_CREAT | 0666
    );

    if (shmid < 0)
    {
        perror("shmget");
        return -1;
    }

    printf("shmid = %d\n", shmid);

    getchar();

    return 0;
}