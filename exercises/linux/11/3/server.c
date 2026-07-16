#include "threadpool.h"
#include "mysocket.h"
#include <signal.h>
#include <errno.h>

user_list_t *head = NULL;
volatile sig_atomic_t is_running = 1;

void signal_handler(int sig)
{
    is_running = 0;
}

int send_response(int sockfd, const char *msg)
{
    if (!msg) return -1;
    return send(sockfd, msg, strlen(msg), 0);
}

int doResponce(void *argp, int sockfd)
{
    user_info_t user_info = *(user_info_t *)argp;

    const char *resp = NULL;
    int exists = (find(head, user_info) == 0);

    switch (user_info.request)
    {
        case 0: // 登录
            resp = exists ? "登录成功" : "用户不存在";
            break;

        case 1: // 注册
            if (exists)
            {
                resp = "用户已存在";
            }
            else
            {
                push_back(&head, user_info);
                resp = "注册成功";
            }
            break;

        default:
            resp = "未知请求类型";
            break;
    }

    return send_response(sockfd, resp);
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
        ssize_t bytes_received = recv(sockc, &user_info, sizeof(user_info_t), 0);

        if (bytes_received == sizeof(user_info_t))
        {
            printf("收到用户名: %s\n", user_info.username);
        }

        if (bytes_received <= 0)
        {
            printf("[%s:%d]已断开\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
            break;
        }

        doResponce(&user_info, sockc);
    }

    close(sockc);
    return NULL;
}

int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s ServerIP ServerPort\n", argv[0]);
        return -1;
    }

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

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
        {
            if (errno == EINTR)
            {
                break;
            }
            continue;
        }
        threadpool_addtask(&pool, doService, (void *)(long)sock_c);
    }

    printf("\n正在关闭主监听套接字...\n");
    close(sock_l);

    printf("线程池正在销毁并等待任务结束...\n");
    threadpool_destroy(&pool);

    printf("文件保存中...\n");
    userinfo_save(head);

    printf("服务器已成功安全退出。\n");
    return 0;
}