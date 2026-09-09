#include <stdio.h>      // printf, perror
#include <stdlib.h>     // atoi, rand, srand
#include <string.h>     // memset, strlen
#include <unistd.h>     // close
#include <time.h>       // time (用于生成随机数种子)
#include <arpa/inet.h>  // inet_pton, htons, ntohs, inet_ntoa
#include <sys/socket.h> // socket, bind, recvfrom, sendto

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    // 1. 检查命令行参数
    if (argc != 3) {
        fprintf(stderr, "用法: %s <IP地址> <端口号>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    // 初始化随机数种子，确保每次运行回复的随机性不同
    srand((unsigned int)time(NULL));

    // 2. 创建 UDP Socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket 创建失败");
        exit(EXIT_FAILURE);
    }

    // 3. 配置服务器地址结构体
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // 将字符串 IP 转换为网络二进制格式
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("无效的 IP 地址");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 4. 绑定地址
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind 绑定失败");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("服务器已启动，正在监听 %s:%d ...\n", ip, port);

    // 5. 定义回复语料库 (对应图片中的 resp 数组)
    const char *resp[] = {"干得漂亮", "你太棒了", "继续加油", "再接再厉"};
    int resp_count = sizeof(resp) / sizeof(resp[0]);

    // 6. 进入主循环处理请求
    while (1) {
        char buffer[BUFFER_SIZE];
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        // 接收数据
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, 
                             (struct sockaddr *)&client_addr, &addr_len);
        
        if (n < 0) {
            perror("recvfrom 接收失败");
            continue; // 出错不退出，继续尝试接收
        }

        // 确保字符串结束符
        buffer[n] = '\0';

        // 打印收到的信息
        printf("[收到] 来自 %s:%d 的消息: %s\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port), 
               buffer);

        // 随机选择一条回复
        int index = rand() % resp_count;
        const char *reply_msg = resp[index];

        // 发送回复
        ssize_t sent_len = sendto(sockfd, reply_msg, strlen(reply_msg), 0, 
                                  (struct sockaddr *)&client_addr, addr_len);
        
        if (sent_len < 0) {
            perror("sendto 发送失败");
        } else {
            printf("[回复] 发送了: %s\n", reply_msg);
        }
    }

    // 注意：由于是 while(1) 死循环，正常情况下不会执行到这里
    // 除非通过信号中断程序
    close(sockfd);
    return 0;
}