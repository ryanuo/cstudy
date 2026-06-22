#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

enum
{
    MSG_RECV = 1,
    MSG_SEND
};

typedef struct msgbuf
{
    long mtype;
    char mtext[64];
} msgbuf_t;

int main(int argc, char **argv)
{
    int msgid = msgget(ftok(".", 'a'), IPC_CREAT | 0666);
    if (msgid < 0)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    msgbuf_t msg = {MSG_RECV};
    memset(msg.mtext, 0, sizeof(msg.mtext));
    msgrcv(msgid, &msg, sizeof(msg.mtext), MSG_RECV, 0);
    printf("接受的消息为: %s\n", msg.mtext);

    msg.mtype = MSG_SEND;
    strncpy(msg.mtext, "hello world test", sizeof(msg.mtext));
    msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
    printf("发送的消息为: %s\n", msg.mtext);

    msgctl(msgid, IPC_RMID, NULL);


    struct msqid_ds buf = {0};
    if (msgctl(msgid, IPC_STAT, &buf) == 0)
    {
        if (buf.msg_qnum == 0)
        {
            msgctl(msgid, IPC_RMID, &buf);
            printf("msg queue is empty\n");
        }
    }
    return 0;
}