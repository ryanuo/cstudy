#include <stdio.h>
#include "dclist.h"

int main(void)
{
    dclist_t *head = NULL;

    int n = 5;   // 小朋友数量
    int num = 3; // 报数值

    for (int i = 1; i <= n; i++)
    {
        dclist_insert(&head, i);
    }

    printf("初始小朋友编号：\n");
    dclist_showall(head);

    dclist_t *cur = head;

    while (head != NULL && head->next != head)
    {
        for (int i = 1; i < num; i++)
        {
            cur = cur->next;
        }

        printf("出列的小朋友：%d\n", cur->data);

        dclist_t *next = cur->next;

        dclist_delete(&head, cur->data);

        cur = next;

        printf("当前剩余：\n");
        dclist_showall(head);
    }

    if (head != NULL)
    {
        printf("最后剩余的小朋友：%d\n", head->data);
    }

    return 0;
}