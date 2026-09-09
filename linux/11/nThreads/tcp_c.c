/*************************************************************************
  > File Name:    tcp_c.c
  > Author:       Terry Yu
  > Description:
  > Created Time: 2026-07-01 16:31:43
 ************************************************************************/

#include "header.h"

void *doRecieve(void *argp)
{
    int sockfd = (int)(long)argp;
    while (1)
    {
        /*3.接收服务器数据*/
        char szbuf[64] = {0};
        ssize_t n = recv(sockfd, szbuf, sizeof(szbuf) - 1, 0);
        szbuf[n] = 0;
        printf("服务器发来消息:%s\n", szbuf);
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage <%s ServerIP ServerPort>\n", argv[0]);
        return -1;
    }
    /*1.创建流式套接字*/
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        return -1;
    }
    /*2.连接服务器*/
    sin_t server = {AF_INET};
    inet_pton(AF_INET, argv[1], &server.sin_addr);
    server.sin_port = htons(atoi(argv[2]));
    socklen_t len = sizeof(sin_t);
    if (-1 == connect(sockfd, (sa_t *)&server, len))
    {
        perror("connect");
        close(sockfd);
        return -1;
    }
    pthread_t id;
    pthread_create(&id, NULL, doRecieve, (void *)(long)sockfd);

    while (1)
    {
        /*4.回复消息给服务器*/
        char buf[64] = {0};
        printf("请输入回复信息:");
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = 0;
        if (strstr(buf, "bye"))
        {
            pthread_cancel(id);
            break;
        }            
        send(sockfd, buf, strlen(buf), 0);
    }
    /*5.关闭套接字*/
    pthread_join(id,NULL);
    close(sockfd);
    return 0;
}