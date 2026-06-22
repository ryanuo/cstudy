#include "mysem.h"

bool mysem_exist(key_t key)
{
    if ((-1 == semget(key, 0, IPC_CREAT | IPC_EXCL)) && (errno == EEXIST))
        return true;
    return false;
}

int mysem_create(key_t key, int nsems)
{
    return semget(key, nsems, IPC_CREAT | 0600);
}

int mysem_init(int semid, int index, int initvalue)
{
    union semun
    {
        int val;               /* Value for SETVAL */
        struct semid_ds *buf;  /*用于获取IPC_STAT, IPC_SET */
        unsigned short *array; /* Array for GETALL, SETALL */
        struct seminfo *__buf; /* Buffer for IPC_INFO  */
    } un;

    un.val = initvalue;
    return semctl(semid, index, SETVAL, un);
}

int mysem_p(int semid, int index)
{
    struct sembuf sops[1] = {index, -1, SEM_UNDO};
    return semop(semid, sops, 1);
}

int mysem_v(int semid, int index)
{
    struct sembuf sops[1] = {index, 1, SEM_UNDO};
    return semop(semid, sops, 1);
}

int mysem_destroy(int semid)
{
    return semctl(semid, 0, IPC_RMID);
}