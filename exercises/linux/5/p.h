#ifndef _P_H_
#define _P_H_

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define MSGPAY_PATH "/Users/ryanuo/tmp/msgpay_key"
#define THREAD_NUM 2

typedef struct msgbuf
{
    long msgtype;
    char msgdata[64];
} msgbuf_t;

#endif