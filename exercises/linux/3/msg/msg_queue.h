#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <sys/msg.h>

// 定义消息类型
#define MSG_TYPE_CLIENT_TO_SERVER 1
#define MSG_TYPE_SERVER_TO_CLIENT 2

// 定义消息结构体
typedef struct msgbuf {
    long mtype;
    char mtext[64];
} msgbuf_t;

#endif