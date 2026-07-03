#include "threadpool.h"
#include "mysocket.h"
#include <signal.h>

user_list_t *head = NULL;
int is_running = 1;

void signal_handler(int sig)
{
    is_running = 0;
}

int doResponce(void *argp, int sockfd)
{
    user_info_t user_info = *(user_info_t *)argp;

    if (user_info.request == 0)
    {
        if (find(head, user_info) == 0)
        {
            char *resp = "登录成功";
            ssize_t l = send(sockfd, resp, strlen(resp), 0);
        }
        else
        {
            char *resp = "用户不存在";
            ssize_t l = send(sockfd, resp, strlen(resp), 0);
        }
    }

    if (user_info.request == 1)
    {
        if (find(head, user_info) == 0)
        {
            char *resp = "用户已存在";
            ssize_t l = send(sockfd, resp, strlen(resp), 0);
        }
        else
        {
            push_back(&head, user_info);
            char *resp = "注册成功";
            ssize_t l = send(sockfd, resp, strlen(resp), 0);
        }
    }
    return 0;
}

void *doService(void *argp)
{
    int sockc = (int)(long)argp;

    sin_t peer = {0};
    socklen_t len = sizeof(sin_t);
    getpeername(sockc, (sa_t *)&peer, &len);

    for (;;)
    {
        user_info_t user_info = {0};
        // 必须循环接收，直到收满 sizeof(user_info_t) 字节
        ssize_t bytes_received = recv(sockc, &user_info, sizeof(user_info_t), 0);

        if (bytes_received == sizeof(user_info_t))
        {
            printf("收到用户名: %s\n", user_info.username);
        }

        if (bytes_received == 0)
        {
            printf("[%s:%d]已断开\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
            break;
        }

        doResponce(&user_info, sockc);
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s ServerIP ServerPort\n", argv[0]);
        return -1;
    }

    signal(SIGINT, signal_handler);

    int sock_l = mysocket_init(argv[1], atoi(argv[2]));
    if (sock_l == -1)
    {
        perror("socket init");
        return -1;
    }
    threadpool_t pool;

    file_init();
    userinfo_load(&head);

    if (-1 == threadpool_init(&pool, 10, 20))
    {
        fprintf(stderr, "threadpool init failure!\n");
        close(sock_l);
        return -1;
    }
    while (is_running)
    {
        int sock_c = mysocket_accept(sock_l);
        if (sock_c == -1)
            continue;
        threadpool_addtask(&pool, doService, (void *)(long)sock_c);
    }

    printf("文件保存中...\n");
    userinfo_save(head);
    threadpool_destroy(&pool);
    close(sock_l);
    return 0;
}