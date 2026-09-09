/*************************************************************************
  > File Name:    header.h
  > Author:       Terry Yu
  > Description:  
  > Created Time: 2026-06-30 16:06:46
 ************************************************************************/

#ifndef _HEADER_H
#define _HEADER_H

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <time.h>
#include <signal.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>

typedef struct sockaddr     sa_t;
typedef struct sockaddr_in  sin_t;


#endif //_HEADER_H