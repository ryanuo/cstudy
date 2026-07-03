#ifndef __HEADER_H__
#define __HEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

typedef struct sockaddr sa_t;
typedef struct sockaddr_in sin_t;

typedef struct
{
    int request; // 0:login, 1:register
    char username[10];
    char password[10];
} user_info_t;

typedef struct user_list
{
    user_info_t data;
    struct user_list *next;
} user_list_t;

typedef user_info_t DataType;
typedef user_list_t Node;

Node *create_node(DataType data);
int push_back(Node **head, DataType data);
int remove_node(Node **head, DataType data);
int find(Node *head, DataType data);
void destroy_list(Node **head);

void file_init(void);
int userinfo_load(user_list_t **head);
int userinfo_save(user_list_t *head);

#endif