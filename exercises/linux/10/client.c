#include "common.h"
#include <net/if.h>
#include <pthread.h>

#define SERVER_IP "10.166.161.68"
#define SERVER_PORT 3006

#define GROUP_IP "224.0.2.100"
#define GROUP_PORT 3006

int sockfd;
int group_sockfd;

sin_t server_addr;

void handle_msg(char *buf)
{
    printf("%s", buf);
    if (strncmp(buf, "SERVER|", 7) == 0)
    {
        printf("\n[服务器回复]\n%s\n", buf + 7);
    }
    else if (strncmp(buf, "GROUP|", 6) == 0)
    {
        printf("\n[群消息]\n%s\n", buf + 6);
    }
    else
    {
        printf("\n[未知消息]\n%s\n", buf);
    }
}

void *recv_server_msg(void *arg)
{
    int fd = *(int *)arg;
    char buf[256];

    while (1)
    {
        memset(buf, 0, sizeof(buf));
        recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);

        handle_msg(buf);
    }
}

void *recv_group_msg(void *arg)
{
    int fd = *(int *)arg;
    char buf[256];

    while (1)
    {
        memset(buf, 0, sizeof(buf));
        recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);

        handle_msg(buf);
    }
}

void init_group_socket()
{
    group_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    int reuse = 1;
    setsockopt(group_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // ===== 1. 绑定组播端口（关键修复）=====
    sin_t addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(GROUP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // ✔ 必须 ANY

    if (bind(group_sockfd, (sa_t *)&addr, sizeof(addr)) < 0)
    {
        perror("bind failed");
    }

    // ===== 2. 加入组播组 =====
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(GROUP_IP);

    // ✔ 推荐稳定写法（选一个）
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    setsockopt(group_sockfd,
               IPPROTO_IP,
               IP_ADD_MEMBERSHIP,
               &mreq,
               sizeof(mreq));
}
void send_msg(const char *msg)
{
    sendto(sockfd, msg, strlen(msg), 0,
           (sa_t *)&server_addr, sizeof(server_addr));
}

void menu()
{
    printf("\n==========\n");
    printf("1. 加入群聊\n");
    printf("2. 离开群聊\n");
    printf("3. 查看群成员\n");
    printf("==========\n");
}

int main()
{
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    init_group_socket();

    pthread_t t1, t2;

    pthread_create(&t1, NULL, recv_server_msg, &sockfd);
    pthread_create(&t2, NULL, recv_group_msg, &group_sockfd);

    char input[16];

    while (1)
    {
        menu();
        printf(">> ");
        scanf("%s", input);

        if (strcmp(input, "1") == 0)
        {
            send_msg("JOIN");
        }
        else if (strcmp(input, "2") == 0)
        {
            send_msg("LEAVE");
        }
        else if (strcmp(input, "3") == 0)
        {
            send_msg("LIST");
        }
    }

    return 0;
}