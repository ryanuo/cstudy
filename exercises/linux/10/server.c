#include "common.h"
#include <net/if.h>

#define SERVER_IP "10.166.161.68"
#define SERVER_PORT 3006

#define GROUP_IP "224.0.2.100"
#define GROUP_PORT 3006

Node *client_list = NULL;

int sockfd;
int mcast_sock;
sin_t group_addr;

void send_group_msg(const char *msg)
{
    sendto(mcast_sock, msg, strlen(msg), 0,
           (sa_t *)&group_addr, sizeof(group_addr));
}

void print_members()
{
    printf("\n==== 当前群成员 ====\n");
    Node *cur = client_list;

    while (cur)
    {
        printf("%s:%d\n", cur->data.ip, cur->data.port);
        cur = cur->next;
    }
    printf("====================\n\n");
}

void handle_join(client_info_t ci)
{
    if (!find(client_list, ci))
    {
        push_back(&client_list, ci);
    }

    char msg[128];
    sprintf(msg, "GROUP|%s:%d 已加群", ci.ip, ci.port);

    printf("[SERVER] %s\n", msg);
    send_group_msg(msg);
}

void handle_leave(client_info_t ci)
{
    remove_node(&client_list, ci);

    char msg[128];
    snprintf(msg, sizeof(msg),
             "%s:%d 已离群", ci.ip, ci.port);

    printf("[SERVER] %s\n", msg);
    send_group_msg(msg);
}

void parse_msg(char *buf, client_info_t *ci)
{
    sscanf(buf, "%15[^:]:%d:%s",
           ci->ip, &ci->port, buf);
}

int main()
{
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    sin_t server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    bind(sockfd, (sa_t *)&server_addr, sizeof(server_addr));

    mcast_sock = socket(AF_INET, SOCK_DGRAM, 0);

    int ttl = 64;
    setsockopt(mcast_sock, IPPROTO_IP,
               IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    memset(&group_addr, 0, sizeof(group_addr));
    group_addr.sin_family = AF_INET;
    group_addr.sin_port = htons(GROUP_PORT);
    group_addr.sin_addr.s_addr = inet_addr(GROUP_IP);

    printf("Server started...\n");

    while (1)
    {
        char buf[256] = {0};
        sin_t cli;
        socklen_t len = sizeof(cli);

        recvfrom(sockfd, buf, sizeof(buf), 0,
                 (sa_t *)&cli, &len);

        client_info_t ci;
        strcpy(ci.ip, inet_ntoa(cli.sin_addr));
        ci.port = ntohs(cli.sin_port);

        if (strcmp(buf, "JOIN") == 0)
        {
            handle_join(ci);
        }
        else if (strcmp(buf, "LEAVE") == 0)
        {
            handle_leave(ci);
        }
        else if (strcmp(buf, "LIST") == 0)
        {
            char msg[1024] = {0};
            char tmp[128];

            Node *cur = client_list;

            strcat(msg, "SERVER|==== 群成员列表 ====\n");

            while (cur)
            {
                snprintf(tmp, sizeof(tmp),
                         "%s:%d\n",
                         cur->data.ip,
                         cur->data.port);

                strcat(msg, tmp);
                cur = cur->next;
            }

            sendto(sockfd, msg, strlen(msg), 0,
                   (sa_t *)&cli, len);
        }
    }

    return 0;
}