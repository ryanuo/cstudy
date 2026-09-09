/*************************************************************************
  > File Name:    hw_s.c
  > Author:       Terry Yu
  > Description:  单文件模块化重构版
  > Created Time: 2026-07-02 10:03:02
 ************************************************************************/

#include "header.h"
#include "slist.h"

#define SERVER_IP INADDR_LOOPBACK
#define SERVER_PORT 10086
#define BUF_SIZE 1024

/* ================= 1. 网络初始化模块 ================= */

/**
 * @brief 初始化并绑定单播 UDP 套接字
 */
static int init_unicast_socket()
{
    int usockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (usockfd == -1)
    {
        perror("socket(unicast)");
        return -1;
    }

    sin_t server = {AF_INET};
    server.sin_addr.s_addr = htonl(SERVER_IP);
    server.sin_port = htons(SERVER_PORT);
    socklen_t len = sizeof(sin_t);

    if (bind(usockfd, (sa_t *)&server, len) == -1)
    {
        perror("bind");
        close(usockfd);
        return -1;
    }
    return usockfd;
}

/**
 * @brief 初始化组播 UDP 套接字及地址结构
 */
static int init_multicast_socket(const char *ip, int port, sin_t *multicast_addr)
{
    int msockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (msockfd == -1)
    {
        perror("socket(multicast)");
        return -1;
    }

    multicast_addr->sin_family = AF_INET;
    multicast_addr->sin_addr.s_addr = inet_addr(ip);
    multicast_addr->sin_port = htons(port);
    return msockfd;
}

/* ================= 2. 消息发送模块 ================= */

/**
 * @brief 向组播地址发送格式化消息
 */
static void send_to_multicast(int msockfd, const sin_t *multicast_addr, const char *msg)
{
    // 1. 安全地将传入的字符串拷贝到发送缓冲区
    char sbuf[BUF_SIZE] = {0};
    strncpy(sbuf, msg, sizeof(sbuf) - 1);

    // 2. 发送数据
    socklen_t len = sizeof(sin_t);
    ssize_t n = sendto(msockfd, sbuf, strlen(sbuf), 0, (sa_t *)multicast_addr, len);

    // 3. 打印发送结果
    printf("[组播] 发送了 %ld 字节: %s\n", n, sbuf);
}

/**
 * @brief 向指定客户端发送单播消息
 */
static void send_to_unicast(int usockfd, const sin_t *peer, const char *msg)
{
    socklen_t len = sizeof(sin_t);
    ssize_t n = sendto(usockfd, msg, strlen(msg), 0, (sa_t *)peer, len);
    printf("[单播] 发送了 %ld 字节\n", n);
}

/* ================= 3. 业务逻辑模块 ================= */

/**
 * @brief 处理客户端发来的各类指令
 */
static void handle_client_request(slist_t **head, int usockfd, int msockfd,
                                  const sin_t *multicast_addr, const sin_t *peer,
                                  const char *msg)
{
    char sbuf[BUF_SIZE] = {0};

    if (strcmp(msg, "已加群") == 0)
    {
        slist_addtail(head, *peer);
        snprintf(sbuf, sizeof(sbuf), "[%s:%d]:已加群!",
                 inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
        send_to_multicast(msockfd, multicast_addr, sbuf);
    }
    else if (strcmp(msg, "已退群") == 0)
    {
        slist_delete(head, *peer);
        snprintf(sbuf, sizeof(sbuf), "[%s:%d]:已退群!",
                 inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
        send_to_multicast(msockfd, multicast_addr, sbuf);
    }
    else if (strcmp(msg, "查看群成员") == 0)
    {
        slist_t *p = *head;
        while (p)
        {
            char peerinfo[64] = {0};
            snprintf(peerinfo, sizeof(peerinfo), "[%s:%d]\n",
                     inet_ntoa(p->data.sin_addr), ntohs(p->data.sin_port));
            strncat(sbuf, peerinfo, sizeof(sbuf) - strlen(sbuf) - 1);
            p = p->next;
        }
        send_to_unicast(usockfd, peer, sbuf);
    }
}

/* ================= 4. 主控制模块 ================= */

int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s 组播地址 组播端口\n", argv[0]);
        return -1;
    }

    // 1. 初始化网络
    int usockfd = init_unicast_socket();
    if (usockfd == -1)
        return -1;

    sin_t multicast_addr = {0};
    int msockfd = init_multicast_socket(argv[1], atoi(argv[2]), &multicast_addr);
    if (msockfd == -1)
    {
        close(usockfd);
        return -1;
    }

    // 2. 初始化业务链表
    slist_t *head = NULL;

    // 3. 事件循环
    while (1)
    {
        sin_t peer = {0};
        char rbuf[64] = {0};
        socklen_t len = sizeof(sin_t);

        ssize_t n = recvfrom(usockfd, rbuf, sizeof(rbuf) - 1, 0, (sa_t *)&peer, &len);
        if (n <= 0)
            continue; // 处理接收异常

        // 4. 调度业务处理
        handle_client_request(&head, usockfd, msockfd, &multicast_addr, &peer, rbuf);
    }

    // 5. 资源清理
    close(usockfd);
    close(msockfd);
    slist_destroy(&head);
    return 0;
}