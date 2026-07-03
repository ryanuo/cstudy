/*************************************************************************
  > File Name:    main.c
  > Author:       Terry Yu
  > Description:
  > Created Time: 2026-07-03 10:45:47
 ************************************************************************/

#include <stdio.h>
#include "threadpool.h"
#include "mysocket.h"

int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s ServerIP ServerPort\n", argv[0]);
        return -1;
    }
    int sockl = mysocket_init(argv[1], atoi(argv[2]));
    if (sockl == -1)
    {
        perror("socket init");
        return -1;
    }
    threadpool_t pool;

    if (-1 == threadpool_init(&pool, 10, 20))
    {
        fprintf(stderr, "threadpool init failure!\n");
        close(sockl);
        return -1;
    }
    while (1)
    {
        int sockc = mysocket_accept(sockl);
        if (sockc == -1)
            continue;
        threadpool_addtask(&pool,doService,(void*)(long)sockc);
    }
    threadpool_destroy(&pool);
    close(sockl);
    return 0;
}
