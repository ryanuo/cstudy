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

int mysocket_accept(int sock_l)
{
    sin_t peer = {0};
    socklen_t len = sizeof(sin_t);

    int sock_c = accept(sock_l, (sa_t *)&peer, &len);
    if (sock_c == -1)
        return -1;
    printf("[%s:%d]已经连接!\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));

    return sock_c;
}

