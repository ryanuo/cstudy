#ifndef __COMMON_H__
#define __COMMON_H__

#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct sockaddr sa_t;
typedef struct sockaddr_in sin_t;

typedef struct
{
    char ip[16];
    int port;
} client_info_t;

typedef struct client_list
{
    client_info_t data;
    struct client_list *next;
} client_list_t;


typedef client_info_t DataType;
typedef client_list_t Node;

Node *create_node(DataType data);
int push_back(Node **head, DataType data);
int remove_node(Node **head, DataType data);

Node *find(Node *head, DataType data);

void destroy_list(Node **head);

#endif