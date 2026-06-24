#include "p.h"

int msgid = -1;

#define MSGTYPE_SEND 2
#define MSGTYPE_RECV 1

void sig_handler(int signo)
{
    (void)signo;
    msgctl(msgid, IPC_RMID, NULL); // 回收消息队列
    exit(EXIT_SUCCESS);
}

void *send_thread(void *arg)
{

    int *peerID = (int *)arg;

    for (;;)
    {
        msgbuf_t msg = {MSGTYPE_SEND};
        printf("请输入要发送的消息:");
        fgets(msg.msgdata, sizeof(msg.msgdata), stdin);
        msg.msgdata[strcspn(msg.msgdata, "\n")] = 0;
        if (strncmp(msg.msgdata, "bye", 3) == 0) // 结束聊天
        {
            kill(*peerID, SIGUSR1);
            kill(getpid(), SIGUSR1);
            break;
        }
        msgsnd(msgid, &msg, strlen(msg.msgdata), 0);
    }

    return NULL;
}

void *recv_thread(void *arg)
{
    while (1)
    {
        msgbuf_t msg = {MSGTYPE_RECV};
        msgrcv(msgid, &msg, sizeof(msg.msgdata) - 1, msg.msgtype, 0);
        printf("对方发来消息:%s\n", msg.msgdata);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    signal(SIGINT, sig_handler);
    msgid = msgget(ftok(MSGPAY_PATH, 'a'), IPC_CREAT | 0666);
    if (msgid < 0)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    msgbuf_t msg = {MSGTYPE_SEND};
    pid_t pid = getpid();
    memcpy(msg.msgdata, &pid, sizeof(pid_t));
    msgsnd(msgid, &msg, sizeof(pid_t), 0);

    msg.msgtype = MSGTYPE_RECV;
    bzero(msg.msgdata, sizeof(msg.msgdata));
    msgrcv(msgid, &msg, sizeof(pid_t), msg.msgtype, 0);
    pid_t peerID;
    memcpy(&peerID, msg.msgdata, sizeof(pid_t));

    pthread_t threads[THREAD_NUM];

    pthread_create(&threads[0], NULL, recv_thread, NULL);
    pthread_create(&threads[1], NULL, send_thread, &peerID);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    return 0;
}