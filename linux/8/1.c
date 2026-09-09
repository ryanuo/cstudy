#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "用法: %s <服务器IP地址> <服务器端口号>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("创建socket失败");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("无效的IP地址");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    socklen_t addr_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];
    ssize_t n;

    printf("已连接到服务器 %s:%d\n", server_ip, server_port);
    printf("请输入消息 (输入 'q' 或 'quit' 退出):\n");

    while (1) {
        // 从标准输入获取用户消息
        printf("> ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break; // 处理EOF (例如 Ctrl+D)
        }

        // 移除 fgets 读入的换行符
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // 检查退出命令
        if (strcmp(buffer, "q") == 0 || strcmp(buffer, "quit") == 0) {
            printf("正在退出...\n");
            break;
        }

        // 发送消息到服务器
        n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, addr_len);
        if (n < 0) {
            perror("发送数据失败");
            break;
        }

        // 接收来自服务器的回复
        n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&server_addr, &addr_len);
        if (n < 0) {
            perror("接收数据失败");
            break;
        }

        // 确保接收到的字符串以 null 结尾
        buffer[n] = '\0';
        printf("服务器回复: %s\n", buffer);
    }

    // 4. 关闭 socket
    close(sockfd);
    return 0;
}