#include "header.h"
#include <termios.h>
#include <sys/select.h>

volatile bool is_login = false;

void menu()
{
    printf("=======================\n");
    printf("  1.    用户注册\n");
    printf("  2.    用户登录\n");
    printf("  0.    退出系统\n");
    printf("=======================\n");
}

void *do_receive(void *argp)
{
    int sockfd = (int)(long)argp;
    while (1)
    {
        char sz_buf[64] = {0};
        ssize_t n = recv(sockfd, sz_buf, sizeof(sz_buf) - 1, 0);

        if (n <= 0)
        {
            if (n == 0)
            {
                printf("\n服务器已断开连接\n");
            }
            else
            {
                perror("recv error");
            }
            break; // 退出接收循环
        }

        sz_buf[n] = 0;
        printf("\n服务器发来消息: %s\n", sz_buf);

        if (strcmp(sz_buf, "登录成功") == 0)
        {
            is_login = true;
            break;
        }
    }

    return NULL;
}

int init_server_socket(char *argv[])
{
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

    return sockfd;
}

/**
 * @brief 获取带掩码(*)的终端输入
 * @param prompt 提示语
 * @param buffer 接收输入的缓冲区
 * @param size   缓冲区大小
 */
static void get_masked_input(const char *prompt, char *buffer, size_t size)
{
    printf("%s", prompt);
    fflush(stdout);

    struct termios oldt, newt;
    int ch, i = 0;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while ((ch = getchar()) != '\n' && ch != EOF && i < size - 1)
    {
        if (ch == 127 || ch == 8)
        {
            if (i > 0)
            {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        }
        else
        {
            buffer[i++] = ch;
            putchar('*');
            fflush(stdout);
        }
    }
    buffer[i] = '\0';

    // 恢复终端原有设置并换行
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}

void user_login(int sockfd)
{
    printf("===== 用户登录 =====\n");

    // 1. 获取用户名 (明文)
    char username[32] = {0};
    printf("请输入用户名: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    // 2. 获取密码 (掩码)
    char password[32] = {0};
    get_masked_input("请输入密码: ", password, sizeof(password));

    user_info_t user_info = {0};
    strncpy(user_info.username, username, sizeof(user_info.username) - 1);
    strncpy(user_info.password, password, sizeof(user_info.password) - 1);
    user_info.request = 0;

    send(sockfd, &user_info, sizeof(user_info), 0);

    printf("✅ 登录信息已发送！\n");
}

void user_register(int sockfd)
{
    printf("===== 用户注册 =====\n");

    // 1. 获取用户名 (明文)
    char username[32] = {0};
    printf("请输入用户名: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    // 2. 获取密码 (掩码)
    char password[32] = {0};
    get_masked_input("请输入密码: ", password, sizeof(password));

    // 3. 获取确认密码 (掩码)
    char confirm_password[32] = {0};
    get_masked_input("请输入确认密码: ", confirm_password, sizeof(confirm_password));

    // 4. 校验密码
    if (strcmp(password, confirm_password) != 0)
    {
        printf("❌ 两次输入的密码不一致，注册失败！\n");
        return;
    }

    user_info_t user_info = {0};
    strncpy(user_info.username, username, sizeof(user_info.username) - 1);
    strncpy(user_info.password, password, sizeof(user_info.password) - 1);
    user_info.request = 1;

    send(sockfd, &user_info, sizeof(user_info), 0);
    printf("✅ 注册信息已发送！\n");
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage <%s ServerIP ServerPort>\n", argv[0]);
        return -1;
    }

    int sockfd = init_server_socket(argv);
    if (sockfd < 0)
    {
        perror("init_server_socket");
        return -1;
    }

    pthread_t id;
    pthread_create(&id, NULL, do_receive, (void *)(long)sockfd);

    bool need_print_menu = true; // 控制是否打印菜单的标志

    while (is_login == false)
    {
        // 1. 只有在需要的时候才打印菜单
        if (need_print_menu)
        {
            menu();
            printf("请输入选择: ");
            fflush(stdout);
            need_print_menu = false; // 打印后重置标志
        }

        // 2. 使用 select 监听标准输入，设置 0.1秒 (100毫秒) 超时
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100毫秒

        int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

        // 3. 如果超时，说明用户没输入，回到 while 顶部检查 is_login，但不打印菜单
        if (ret == 0)
        {
            continue;
        }
        // 4. 如果发生错误，退出
        else if (ret < 0)
        {
            perror("select error");
            break;
        }

        // 5. 如果用户真的输入了，才去读取并处理
        int choice;
        if (scanf("%d", &choice) == 1)
        {
            getchar(); // 吸收回车

            if (choice == 0)
            {
                printf("退出系统\n");
                break;
            }

            switch (choice)
            {
            case 1:
                user_register(sockfd);
                break;
            case 2:
                user_login(sockfd);
                break;
            default:
                printf("无效的选择！\n");
            }

            // 6. 处理完一次用户输入后，下一次循环再打印菜单
            need_print_menu = true;
        }
    }

    pthread_join(id, NULL);
    close(sockfd);
    return 0;
}