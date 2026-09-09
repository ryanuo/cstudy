#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

struct DownloadReq
{
    char cmd[4];       // 指令，如 "GET"
    char filename[60];
};

typedef struct sockaddr_in sock_t;
#endif