#include "slist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/***
@function:  创建单向链表
@argment:
          head: 用于接收头指针的地址
          data: 节点数据
@retval:  成功:  0
          失败:  -1
@author:  class2604 
***/

int slist_create(slist_t **head,slist_data_t  data)
{
    slist_t *pnew = (slist_t*) malloc(sizeof(slist_t));
    if(pnew == NULL)
	return  -1;      
    pnew -> data  = data;
    pnew -> next  = NULL;

    *head = pnew;
    return   0;
}

/***
@function: 向单向链表头部添加数据节点
@argment:
          head: 用于接收头指针的地址
          data: 节点数据
@retval:  成功:  0
          失败:  -1
@author:  class2604 
***/

int  slist_addhead(slist_t** head,slist_data_t data)
{
    slist_t *pnew = (slist_t*) malloc(sizeof(slist_t));
    if(pnew == NULL)
        return  -1;      
    pnew -> data  = data;
    pnew -> next  = *head;

    *head = pnew;
    return 0;
}
/***
@function: 向单向链表尾部添加数据节点
@argment:
          head: 用于接收头指针的地址
          data: 节点数据
@retval:  成功:  0
          失败:  -1
@author:  class2604 
***/

int  slist_addtail(slist_t** head,slist_data_t data)
{
    slist_t *pnew = (slist_t*) malloc(sizeof(slist_t));
    if(pnew == NULL)
        return  -1;      
    pnew -> data  = data;
    pnew -> next  = NULL;

    slist_t *p = *head, *q = NULL;
    while(p)
    { 
         q = p;
         p = p -> next;
    }
    q ? (q -> next = pnew) : (*head = pnew);

    return 0;
}

int  slist_insert(slist_t** head,slist_data_t pos,slist_data_t new)
{
    slist_t *pnew = (slist_t*) malloc(sizeof(slist_t));
    if(pnew == NULL)
        return  -1;
    pnew -> data  = new;
    pnew -> next  = NULL;

    slist_t *p = *head, *q = NULL;
    while(p)
    {
        if(memcmp(&p -> data, &pos,sizeof(slist_data_t)) == 0)
        {
              pnew -> next  = p;
              q? (q-> next  = pnew):(*head = pnew);
              return 0;
        }
        q  = p;
        p  = p -> next;
    }
    q? (q-> next  = pnew):(*head = pnew);

    return 0;
}

int  slist_delete(slist_t** head,slist_data_t data)
{
    slist_t *p = *head, *q = NULL;

    while(p)
    {
         if(memcmp(&p->data,&data,sizeof(slist_data_t)) == 0)
         {
              q ? (q -> next = p -> next):(*head = p -> next);
              free(p);
              return  0;
         }
         q  = p ;
         p  = p -> next;
    }    
    return -1;    
}
slist_t*  slist_query(const slist_t* head,slist_data_t data)
{
     const slist_t *p = head;
     while(p)
     {
          if(memcmp(&p->data,&data,sizeof(slist_data_t)) == 0)
               return (slist_t*)p;
          p = p -> next;
     }
     return NULL;
}
int slist_update(const slist_t *head,slist_data_t old,slist_data_t new)
{
     slist_t *p  = slist_query(head,old);
     if(p == NULL)
       return -1;
     p -> data = new;
     return 0;
}

int  slist_destroy(slist_t **head)
{
    slist_t *p = *head,*q = NULL;
    while(p)
    {
        q  = p;
        p  = p -> next;
        free(q);
    }
    *head = NULL; 
    return 0;
}
/***
@function: 遍历单向链表数据节点
@argment:
          head: 用于接收头指针
@retval:  0
@author:  class2604 
***/

int  slist_showall(const slist_t* head)
{
     const slist_t *p = head;
     while(p)
     {
        //   printf("%d ",p->data);
          p = p -> next; 
     }
     printf("\n");
     return 0;
}
