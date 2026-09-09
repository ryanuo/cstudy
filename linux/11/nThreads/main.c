/*************************************************************************
  > File Name:    main.c
  > Author:       Terry Yu
  > Description:  
  > Created Time: 2026-07-02 14:33:58
 ************************************************************************/

#include "mysocket.h"

int main(int argc,char const *argv[])
{
    if(argc < 3)
    {
        fprintf(stderr,"Usage: %s ServerIP ServerPort\n",argv[0]);
        return -1;
    }
    int sockl  = mysocket_init(argv[1],atoi(argv[2]));
    if(sockl == -1)
    {
        perror("socket init");
        return -1;
    }
    while(1)
    {
        int sockc  = mysocket_accept(sockl); 
        if(sockc == -1)
           continue; 

        pthread_t  id;
        pthread_create(&id,NULL,doService,(void*)(long)sockc);
        pthread_detach(id);
    }
    close(sockl);
    return 0;
}