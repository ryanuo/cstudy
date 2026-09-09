#include "common.h"

/**
 * @brief 客户端程序
 * @serverIP 服务器IP
 * @port 端口号
 * @RemoteFile 指定下载的文件名
 * @saveFile 保存文件名
 */
int main(int argc, char **argv)
{
    if (argc != 5)
    {
        printf("Usage: %s <ServerIP> <Port> <RemoteFile> <SaveFile>\n", argv[0]);
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    sock_t server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    struct DownloadReq req = {0};
    strcpy(req.cmd, "GET");
    strcpy(req.filename, argv[3]);

    sendto(sockfd, &req, sizeof(req), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("已向服务器请求下载文件: %s\n", req.filename);

    uint32_t file_size_net = 0;
    socklen_t server_addr_len = sizeof(server_addr);
    recvfrom(sockfd, &file_size_net, sizeof(file_size_net), 0, (struct sockaddr *)&server_addr, &server_addr_len);

    uint32_t file_size = ntohl(file_size_net);
    printf("准备接收文件，大小: %d bytes\n", file_size);

    FILE *fp = fopen(argv[4], "wb");
    if (fp == NULL)
    {
        perror("fopen");
        exit(1);
    }

    char buf[1024];
    uint32_t received_size = 0;
    ssize_t n;

    while (received_size < file_size)
    {
        n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (n < 0)
        {
            perror("recvfrom");
            close(sockfd);
            exit(1);
        }

        fwrite(buf, 1, n, fp);
        received_size += n;

        printf("\r接收进度: %d / %d bytes", received_size, file_size);
    }

    printf("\n接收完成\n");
    
    fclose(fp);
    close(sockfd);

    return 0;
}