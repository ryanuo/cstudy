#ifndef SV_CHAT_SINGLE_H
#define SV_CHAT_SINGLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define KEY_PATH "/Users/ryanuo/tmp/sv_chat_single_key"
#define PROJ_ID 'C'
#define MSG_TEXT_SIZE 256

// ⭐ 消息类型约定（绝对不能搞混）
#define MTYPE_FROM_A 1
#define MTYPE_FROM_B 2

typedef struct
{
    long mtype;
    pid_t sender_pid;
    char text[MSG_TEXT_SIZE];
} ChatMsg;

extern volatile sig_atomic_t peer_exited;
#endif