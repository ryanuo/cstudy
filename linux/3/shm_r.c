#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include "mysem.h"

#define KEY 0x04030201

int main(int argc, char const *argv[])
{
    bool exist = false;
    if (mysem_exist((key_t)KEY))
        exist = true;

    int semid = mysem_create((key_t)KEY, 2);
    if (semid == -1)
    {
        perror("mysem_create");
        exit(EXIT_FAILURE);
    }

    if (exist == false)
    {
        mysem_init(semid, 0, 0);
        mysem_init(semid, 1, 1);
    }

    /*1.申请创建共享内存，并将新共享内存的键值设为0x01020304*/
    int shmid = shmget((key_t)0x01020304, 1024, IPC_CREAT | 0644);
    if (shmid == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    /*2.映射共享内存到进程地址空间 */
    char *paddr = (char *)shmat(shmid, NULL, 0);
    if (paddr == MAP_FAILED)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    /*3.操作映射后的内存*/
    for (register int i = 0; i < 10; i++)
    {
        mysem_p(semid, 0);
        puts(paddr);
        mysem_v(semid, 1);
    }

    /*4.解除映射*/
    shmdt(paddr);

    /*5回收共享内存*/
    shmctl(shmid, IPC_RMID, NULL);
    mysem_destroy(semid);
    return 0;
}
