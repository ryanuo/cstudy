#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <BroadcastPort>\n", argv[0]);
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    // 修正：使用标准的 struct sockaddr_in
    struct sockaddr_in any;
    memset(&any, 0, sizeof(any));
    any.sin_family = AF_INET;
    any.sin_port = htons(atoi(argv[1]));
    any.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&any, sizeof(any)) < 0)
    {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    printf("等待接收广播数据...\n");
    char szbuf[64] = {0};
    struct sockaddr_in peer;
    socklen_t len = sizeof(peer);

    // 修正：加入循环，使其能持续接收
    while (1)
    {
        ssize_t n = recvfrom(sockfd, szbuf, sizeof(szbuf) - 1, 0, 
                             (struct sockaddr *)&peer, &len);
        if (n < 0)
        {
            perror("recvfrom");
            continue; // 出错不退出，继续接收
        }

        szbuf[n] = '\0';
        
        // 修正：使用 inet_ntop 替代 inet_ntoa，更安全
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &peer.sin_addr, ip_str, sizeof(ip_str));

        printf("[%s:%d 广播数据] %s\n",
               ip_str,
               ntohs(peer.sin_port),
               szbuf);

        // 回复发送方（单播回复，安全）
        char buf[64] = {0};
        printf("请输入要发送的数据：");
        if (fgets(buf, sizeof(buf), stdin) == NULL) break;
        
        // 去除换行符
        buf[strcspn(buf, "\n")] = '\0'; 

        if (strlen(buf) > 0) {
            sendto(sockfd, buf, strlen(buf), 0, 
                   (struct sockaddr *)&peer, sizeof(peer));
        }
    }

    close(sockfd);
    return 0;
}