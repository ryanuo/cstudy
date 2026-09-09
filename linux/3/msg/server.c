#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "msg_queue.h"

int main() {
    key_t key = ftok(".", 'a');
    // 服务端负责创建消息队列
    int msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (msgid < 0) {
        perror("msgget create failed");
        exit(EXIT_FAILURE);
    }

    msgbuf_t msg = {0};
    while (1) {
        // 1. 阻塞等待接收来自客户端的消息 (类型1)
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), MSG_TYPE_CLIENT_TO_SERVER, 0) < 0) {
            perror("msgrcv");
            break;
        }
        printf("[Server] 收到客户端消息: %s\n", msg.mtext);

        // 如果收到 "quit"，则退出循环
        if (strcmp(msg.mtext, "quit") == 0) break;

        // 2. 构造回复消息，并发送给客户端 (类型2)
        msg.mtype = MSG_TYPE_SERVER_TO_CLIENT;
        printf("[Server] 请输入回复: ");
        fgets(msg.mtext, sizeof(msg.mtext), stdin);
        msg.mtext[strcspn(msg.mtext, "\n")] = 0; // 去除换行符

        if (msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0) < 0) {
            perror("msgsnd");
            break;
        }
    }

    // 清理消息队列
    msgctl(msgid, IPC_RMID, NULL);
    printf("[Server] 消息队列已销毁，退出。\n");
    return 0;
}