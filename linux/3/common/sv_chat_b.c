#include "sv_chat_single.h"

volatile sig_atomic_t peer_exited = 0;
void sig_handler(int sig) { peer_exited = 1; }

int main()
{
    signal(SIGUSR1, sig_handler);
    pid_t my_pid = getpid();

    FILE *f = fopen(KEY_PATH, "w");
    fclose(f);
    key_t key = ftok(KEY_PATH, PROJ_ID);

    // ⭐ B 不创建队列，轮询等待 A 创建
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

    // 阻塞等待 A 的握手消息，获取 A 的 PID
    ChatMsg msg;
    msgrcv(qid, &msg, MSG_TEXT_SIZE + sizeof(pid_t), MTYPE_FROM_A, 0);
    pid_t peer_pid = msg.sender_pid;
    printf("[B] Connected! Peer PID: %d\n", peer_pid);

    // 发送握手回复
    msg.mtype = MTYPE_FROM_B;
    msg.sender_pid = my_pid;
    strcpy(msg.text, "HELLO");
    msgsnd(qid, &msg, sizeof(pid_t) + strlen("HELLO") + 1, 0);

    // 聊天主循环
    while (!peer_exited)
    {
        // ⭐ B 只接收 A 发的消息 (MTYPE_FROM_A)，这里用阻塞等待
        ssize_t n = msgrcv(qid, &msg, MSG_TEXT_SIZE + sizeof(pid_t), MTYPE_FROM_A, 0);
        if (n > 0 && !peer_exited)
            printf("[A]: %s", msg.text);
        if (peer_exited)
            break;

        printf("[B] > ");
        fflush(stdout);
        char buf[MSG_TEXT_SIZE];
        if (fgets(buf, sizeof(buf), stdin) == NULL || strncmp(buf, "quit", 4) == 0)
        {
            kill(peer_pid, SIGUSR1);
            break;
        }

        // ⭐ B 发送时必须标记为 MTYPE_FROM_B
        msg.mtype = MTYPE_FROM_B;
        msg.sender_pid = my_pid;
        strncpy(msg.text, buf, MSG_TEXT_SIZE - 1);
        msgsnd(qid, &msg, sizeof(pid_t) + strlen(buf) + 1, 0);
    }

    // ⭐ B 负责最终回收唯一的队列和 key 文件
    msgctl(qid, IPC_RMID, NULL);
    unlink(KEY_PATH);
    printf("[B] Queue cleaned up. Bye.\n");
    return 0;
}