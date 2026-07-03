/*************************************************************************
  > File Name:    mysocket.h
  > Author:       Terry Yu
  > Description:  
  > Created Time: 2026-07-02 14:28:15
 ************************************************************************/

#ifndef _MYSOCKET_H
#define _MYSOCKET_H

#include "header.h"

int mysocket_init(const char* IP, uint16_t Port);
int mysocket_accept(int sockl);
void* doService(void* argp);

#endif //_MYSOCKET_H