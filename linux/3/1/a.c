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

    int qid = msgget(key, IPC_CREAT | 0644);

    if (qid == -1)
    {
        perror("A: msgget");
        exit(1);
    }
    printf("[A] Waiting for B...\n");

    // 发送 pid 给 B
    ChatMsg msg;
    msg.mtype = MTYPE_FROM_A;
    msg.sender_pid = my_pid;
    strcpy(msg.text, "HELLO");
    msgsnd(qid, &msg, sizeof(pid_t) + strlen("HELLO") + 1, 0);

    // 接收 B 的 pid
    msgrcv(qid, &msg, sizeof(pid_t) + MSG_TEXT_SIZE, MTYPE_FROM_B, 0);
    pid_t peer_pid = msg.sender_pid;
    printf("[A] Connected! Peer PID: %d\n", peer_pid);

    int skill_flag = 0;
    while (1)
    {
        // 1. 从标准输入读取用户输入
        printf("[A] 请输入发送给 B 的消息: ");
        fgets(msg.text, sizeof(msg.text), stdin);
        msg.text[strcspn(msg.text, "\n")] = 0; // 去除换行符

        // 2. 发送消息给B (类型1)
        msg.mtype = MTYPE_FROM_A;
        if (msgsnd(qid, &msg, sizeof(pid_t) + strlen(msg.text) + 1, 0) < 0)
        {
            perror("msgsnd");
            break;
        }

        if (strcmp(msg.text, "quit") == 0)
        {
            printf("[A] 退出...\n");
            skill_flag = 0;
            break;
        }

        // 3. 阻塞等待B的回复 (类型2)
        if (msgrcv(qid, &msg, sizeof(pid_t) + sizeof(msg.text), MTYPE_FROM_B, 0) < 0)
        {
            perror("msgrcv");
            break;
        }
        printf("[A] 收到 B 回复: %s\n", msg.text);

        if (strcmp(msg.text, "quit") == 0)
        {
            printf("[A] 退出...\n");
            skill_flag = 1;
            break;
        }
    }

    if (skill_flag)
    {
        msgctl(qid, IPC_RMID, NULL);
        kill(peer_pid, SIGUSR1);
        printf("进程 B 已被杀死.\n");
    }

    return 0;
}