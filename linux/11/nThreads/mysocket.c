#include "mysocket.h"

int mysocket_init(const char *IP, uint16_t Port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        return -1;

    sin_t server = {AF_INET};
    inet_pton(AF_INET, IP, &server.sin_addr);
    server.sin_port = htons(Port);
    socklen_t len = sizeof(sin_t);
    if (-1 == bind(sockfd, (sa_t *)&server, len))
    {
        close(sockfd);
        return -1;
    }
    if (-1 == listen(sockfd, 5))
    {
        close(sockfd);
        return -1;
    }
    return sockfd;
}

int mysocket_accept(int sockl)
{
    sin_t peer = {0};
    socklen_t len = sizeof(sin_t);

    int sockc = accept(sockl, (sa_t *)&peer, &len);
    if (sockc == -1)
        return -1;
    printf("[%s:%d]已经连接!\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));

    return sockc;
}

void *doService(void *argp)
{
    int doResponce(void *argp, int sockfd);
    int sockc = (int)(long)argp;

    sin_t peer = {0};
    socklen_t len = sizeof(sin_t);
    getpeername(sockc, (sa_t *)&peer, &len);

    for (;;)
    {
        char rbuf[64] = {0};
        ssize_t n = recv(sockc, rbuf, sizeof(rbuf) - 1, 0);
        if (n == 0)
        {
            printf("[%s:%d]已断开\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
            break;
        }
        rbuf[n] = 0;
        printf("[%s:%d]发来消息:%s\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port), rbuf);

        doResponce(rbuf, sockc);
    }
    return NULL;
}

int doResponce(void *argp, int sockfd)
{
    const char *resp[] = {"干得漂亮", "你太棒了", "继续加油", "再接再厉"};
    int n = sizeof resp / sizeof resp[0];
    int i = rand() % n;

    ssize_t l = send(sockfd, resp[i], strlen(resp[i]), 0);
}
