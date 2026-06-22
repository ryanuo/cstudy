#include "common.h"

void sig_handler(int sig)
{
    switch (sig)
    {
    case SIGUSR1:
        exit(EXIT_SUCCESS);
        break;

    default:
        break;
    }
}

int main(int argc, char **argv)
{
    signal(SIGUSR1, sig_handler);
    pid_t my_pid = getpid();

    FILE *f = fopen(KEY_PATH, "w");
    fclose(f);
    key_t key = ftok(KEY_PATH, PROJ_ID);

    int qid;
    printf("[B] Waiting for A...\n");
    while ((qid = msgget(key, 0)) == -1)
    {
        if (errno == ENOENT)
            usleep(100000);
        else
        {
            perror("B: msgget");
            exit(1);
        }
    }

    // Receive message from A
    ChatMsg msg;
    msgrcv(qid, &msg, MSG_TEXT_SIZE + sizeof(pid_t), MTYPE_FROM_A, 0);
    pid_t peer_pid = msg.sender_pid;
    printf("[B] Connected! Peer PID: %d\n", peer_pid);

    // Send message to A
    msg.mtype = MTYPE_FROM_B;
    msg.sender_pid = my_pid;
    msgsnd(qid, &msg, MSG_TEXT_SIZE + sizeof(pid_t), 0);

    while (1)
    {
        // 1. 阻塞等待接收来自A的消息 (类型1)
        if (msgrcv(qid, &msg, sizeof(msg.text), MTYPE_FROM_A, 0) < 0)
        {
            perror("msgrcv");
            break;
        }
        printf("[B] 收到 A 消息: %s\n", msg.text);

        // 如果收到 "quit"，则退出循环
        if (strcmp(msg.text, "quit") == 0)
        {
            printf("[B] 退出...\n");
            break;
        }

        // 2. 构造回复消息，并发送给A (类型2)
        msg.mtype = MTYPE_FROM_B;
        printf("[B] 请输入回复: ");
        fgets(msg.text, sizeof(msg.text), stdin);
        msg.text[strcspn(msg.text, "\n")] = 0; // 去除换行符

        if (msgsnd(qid, &msg, sizeof(pid_t) + strlen(msg.text) + 1, 0) < 0)
        {
            perror("msgsnd");
            break;
        }

        // 如果收到 "quit"，则退出循环
        if (strcmp(msg.text, "quit") == 0)
        {
            printf("[B] 退出...\n");
            break;
        }
    }

    msgctl(qid, IPC_RMID, NULL);
    unlink(KEY_PATH);
    kill(peer_pid, SIGUSR1);
    printf("进程 A 已被杀死.\n");
    return 0;
}