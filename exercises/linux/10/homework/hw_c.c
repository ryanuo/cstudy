/*************************************************************************
  > File Name:    hw_c.c
  > Author:       Terry Yu
  > Description:
  > Created Time: 2026-07-02 10:38:16
 ************************************************************************/

#include "header.h"
#include "pthread.h"

#define SERVER_IP INADDR_LOOPBACK
#define SERVER_PORT 10086

bool ismember = false;
int isquit = false;

void menu(void)
{
    puts("=================");
    puts("===1.加入群聊====");
    puts("===2.离开群聊=====");
    puts("===3.查看成员====");
    puts("===0.退出系统====");
    puts("=================");

    printf("\n\n请选择功能:");
}

int init_socket(int port)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        return -1;
    }
    sin_t any = {AF_INET};
    any.sin_addr.s_addr = htonl(INADDR_ANY);
    any.sin_port = htons(port);
    if (-1 == bind(sockfd, (sa_t *)&any, sizeof(sin_t)))
    {
        perror("bind");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

/**
 * @brief 处理加入或退出组播组的逻辑
 * @param sockfd      套接字描述符
 * @param group_ip    组播IP地址字符串 (如 "239.1.1.1")
 * @param type        操作类型: 1=加入, 0=退出
 * @param ismember    指向成员状态变量的指针 (用于在函数内更新状态)
 * @return            0=成功, -1=失败
 */
int group_operate(int sockfd, const char *group_ip, int type)
{
    int is_add = (type == 1);

    // 1. 状态校验
    if (ismember && is_add)
    {
        puts("你已经是群成员了");
        return 0;
    }
    else if (!(ismember) && !is_add)
    {
        puts("你不是群成员");
        return 0;
    }

    // 2. 配置组播请求结构体
    struct ip_mreqn reqn = {0};
    if (inet_pton(AF_INET, group_ip, &reqn.imr_multiaddr) != 1)
    {
        fprintf(stderr, "无效的组播IP地址: %s\n", group_ip);
        return -1;
    }

    inet_pton(AF_INET, "192.168.14.52", &reqn.imr_address);
    reqn.imr_ifindex = if_nametoindex("ens33");

    // 3. 设置套接字选项 (加入/离开组播组)
    if (-1 == setsockopt(sockfd, IPPROTO_IP,
                         is_add ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP,
                         &reqn, sizeof(reqn)))
    {
        perror(is_add ? "Join Group" : "Leave Group");
        return -1;
    }

    // 4. 更新本地成员状态
    ismember = is_add;

    // 5. 向服务器发送通知
    sin_t server = {AF_INET};
    server.sin_addr.s_addr = htonl(SERVER_IP);
    server.sin_port = htons(SERVER_PORT);
    socklen_t len = sizeof(sin_t);

    const char *msg = is_add ? "已加群" : "已退群";
    ssize_t n = sendto(sockfd, msg, strlen(msg), 0, (sa_t *)&server, len);
    if (n == -1)
    {
        perror("sendto server");
        return -1;
    }

    printf("%s 成功\n", msg);
    return 0;
}

// 接收组播的信号
void *recv_thread(void *arg)
{
    int sockfd = *(int *)arg;
    fd_set readfds;
    struct timeval timeout;

    while (!isquit)
    {
        // 只有在加入群聊时才去尝试接收数据
        if (ismember)
        {
            // 1. 清空集合，并将 sockfd 加入监听集合
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);

            // 2. 设置超时时间为 1 秒
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            // 3. 使用 select 进行监听，最多等待 1 秒
            int ret = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

            if (ret > 0 && FD_ISSET(sockfd, &readfds))
            {
                // 有数据到达，可以安全地读取，不会阻塞
                sin_t peer = {0};
                char rbuf[64] = {0};
                socklen_t len = sizeof(sin_t);

                ssize_t n = recvfrom(sockfd, rbuf, sizeof(rbuf) - 1, 0, (sa_t *)&peer, &len);
                if (n > 0)
                {
                    printf("[%s:%d]:发来组播数据:%s\n",
                           inet_ntoa(peer.sin_addr), ntohs(peer.sin_port), rbuf);
                }
            }
            // 如果 ret == 0，说明 1 秒超时，循环继续，顺便检查 isquit
            // 如果 ret < 0，说明发生错误，也可以在此处处理
        }
        else
        {
            // 如果还没加群，休眠 100 毫秒，避免死循环消耗 CPU
            usleep(100000);
        }
    }

    return NULL;
}

void view_group(int sockfd)
{
    if (!ismember)
    {
        printf("\n请先加入群组!!!!!!!\n");
        return;
    }

    // 1. 向服务器发送“查看群成员”请求
    sin_t server = {AF_INET};
    server.sin_addr.s_addr = htonl(SERVER_IP);
    server.sin_port = htons(SERVER_PORT);
    socklen_t len = sizeof(sin_t);

    const char *p = "查看群成员";
    if (sendto(sockfd, p, strlen(p), 0, (sa_t *)&server, len) == -1)
    {
        perror("sendto view_group");
        return;
    }

    // 2. 使用 select 等待服务器响应，设置 3 秒超时
    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    timeout.tv_sec = 3; // 最多等待 3 秒
    timeout.tv_usec = 0;

    int ret = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

    if (ret > 0 && FD_ISSET(sockfd, &readfds))
    {
        // 有数据到达，安全读取
        char buf[1024] = {0};
        socklen_t recv_len = sizeof(sin_t);
        ssize_t n = recvfrom(sockfd, buf, sizeof(buf) - 1, 0, (sa_t *)&server, &recv_len);
        if (n > 0)
        {
            printf("\n===== 当前群成员列表 =====\n%s\n==========================\n", buf);
        }
    }
    else if (ret == 0)
    {
        // 3 秒超时，服务器未响应
        printf("\n[提示] 等待服务器响应超时，请稍后重试...\n");
    }
    else
    {
        // select 发生错误
        perror("select view_group");
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s 组播地址 组播端口\n", argv[0]);
        return -1;
    }

    int sockfd = init_socket(atoi(argv[2]));
    socklen_t len = sizeof(sin_t);

    pthread_t pid;
    pthread_create(&pid, NULL, recv_thread, &sockfd);

    while (1)
    {
        menu();
        int i = -1;
        scanf("%d", &i);
        if (i == 0)
        {
            isquit = true;
            break;
        }
        switch (i)
        {
        case 1:
            group_operate(sockfd, argv[1], 1);
            break;
        case 2:
            group_operate(sockfd, argv[1], 0);
            break;
        case 3:
            view_group(sockfd);
            break;

        default:
            puts("功能开发中,敬请期待...");
            break;
        }
    }

    close(sockfd);
    return 0;
}