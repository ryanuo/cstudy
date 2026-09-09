#include "sv_chat_single.h"

volatile sig_atomic_t peer_exited = 0;
void sig_handler(int sig) { peer_exited = 1; }

int main()
{
    signal(SIGUSR1, sig_handler);
    pid_t my_pid = getpid();

    // 确保 key 文件存在
    FILE *f = fopen(KEY_PATH, "w");
    fclose(f);
    key_t key = ftok(KEY_PATH, PROJ_ID);

    // ⭐ A 作为主导方，负责创建唯一的队列
    int qid = msgget(key, IPC_CREAT | 0644);
    if (qid == -1)
    {
        perror("A: msgget");
        exit(1);
    }

    // 发送握手消息，告诉 B 我的 PID
    ChatMsg msg;
    msg.mtype = MTYPE_FROM_A;
    msg.sender_pid = my_pid;
    strcpy(msg.text, "HELLO");
    msgsnd(qid, &msg, sizeof(pid_t) + strlen("HELLO") + 1, 0);

    // 阻塞等待 B 的握手回复，从中获取 B 的 PID
    msgrcv(qid, &msg, MSG_TEXT_SIZE + sizeof(pid_t), MTYPE_FROM_B, 0);
    pid_t peer_pid = msg.sender_pid;
    printf("[A] Connected! Peer PID: %d\n", peer_pid);

    while (!peer_exited)
    {
        // 只接收 B 发的消息 (MTYPE_FROM_B)
        ssize_t n = msgrcv(qid, &msg, MSG_TEXT_SIZE + sizeof(pid_t), MTYPE_FROM_B, IPC_NOWAIT);
        if (n > 0 && !peer_exited)
            printf("[B]: %s", msg.text);

        printf("[A] > ");
        fflush(stdout);
        char buf[MSG_TEXT_SIZE];
        if (fgets(buf, sizeof(buf), stdin) == NULL || strncmp(buf, "quit", 4) == 0)
        {
            kill(peer_pid, SIGUSR1);
            break;
        }

        // ⭐ A 发送时必须标记为 MTYPE_FROM_A
        msg.mtype = MTYPE_FROM_A;
        msg.sender_pid = my_pid;
        strncpy(msg.text, buf, MSG_TEXT_SIZE - 1);
        msgsnd(qid, &msg, sizeof(pid_t) + strlen(buf) + 1, 0);
    }

    // ⭐ A 不删除队列，直接退出
    printf("[A] Bye.\n");
    return 0;
}