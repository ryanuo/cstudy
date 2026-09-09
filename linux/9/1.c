#include "common.h"

#define BUF_SIZE 1024

/**
 * 服务端主程序
 * @port 端口号
 */
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <Port>\n", argv[0]);
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    sock_t server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(atoi(argv[1])),
        .sin_addr.s_addr = htonl(INADDR_ANY)};

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        return -1;
    }

    sock_t client_addr;
    socklen_t client_len = sizeof(client_addr);

    char buf[BUF_SIZE];

    struct DownloadReq req;

    recvfrom(sockfd, &req, sizeof(req), 0, (struct sockaddr *)&client_addr, &client_len);
    printf("收到客户端的请求：指令[%s],文件=[%s]\n", req.cmd, req.filename);

    FILE *fp = fopen(req.filename, "rb");
    if (fp == NULL)
    {
        perror("fopen");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    uint32_t file_size = htonl(ftell(fp));
    fseek(fp, 0, SEEK_SET);

    sendto(sockfd, &file_size, sizeof(file_size), 0, (struct sockaddr *)&client_addr, client_len);

    size_t nread;
    while ((nread = fread(buf, 1, sizeof(buf), fp)) > 0)
    {
        sendto(sockfd, buf, nread, 0, (struct sockaddr *)&client_addr, client_len);
    }

    printf("File sent Success\n");
    fclose(fp);
    close(sockfd);

    return 0;
}