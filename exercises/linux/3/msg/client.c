#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "msg_queue.h"

int main() {
    key_t key = ftok(".", 'a');
    // 客户端只负责获取消息队列，不创建
    int msgid = msgget(key, 0666);
    if (msgid < 0) {
        perror("msgget open failed (请先启动server)");
        exit(EXIT_FAILURE);
    }

    msgbuf_t msg = {0};
    while (1) {
        // 1. 从标准输入读取用户输入
        printf("[Client] 请输入发送给服务端的消息: ");
        fgets(msg.mtext, sizeof(msg.mtext), stdin);
        msg.mtext[strcspn(msg.mtext, "\n")] = 0; // 去除换行符

        // 2. 发送消息给服务端 (类型1)
        msg.mtype = MSG_TYPE_CLIENT_TO_SERVER;
        if (msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0) < 0) {
            perror("msgsnd");
            break;
        }

        if (strcmp(msg.mtext, "quit") == 0) break;

        // 3. 阻塞等待服务端的回复 (类型2)
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), MSG_TYPE_SERVER_TO_CLIENT, 0) < 0) {
            perror("msgrcv");
            break;
        }
        printf("[Client] 收到服务端回复: %s\n", msg.mtext);
    }

    printf("[Client] 退出。\n");
    return 0;
}