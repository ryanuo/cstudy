#include <stdio.h>
#include <stdlib.h>
#include "generic_list.h"

/*
 * 链表反转测试
 * 使用通用链表接口测试整数数据
 */

/* 输出整数 */
void print_int(const void *data)
{
    if (data == NULL)
        return;
    printf("%d ", *(int *)data);
}

/* 反转链表 */
void generic_slist_reverse(generic_list_t **head)
{
    if (head == NULL || *head == NULL || (*head)->next == NULL)
    {
        return;
    }

    generic_list_t *p = (*head)->next;

    (*head)->next = NULL;

    while (p)
    {
        generic_list_t *temp = p;

        p = p->next;

        temp->next = *head;

        *head = temp;
    }
}

int main(int argc, char **argv)
{
    generic_list_t *head = NULL;

    /* 添加 10 个整数到链表 */
    for (int i = 0; i < 10; i++)
    {
        int value = i * 100;
        slist_add_head(&head, &value, sizeof(int));
    }

    /* 反转前 */
    printf("反转前: ");
    slist_print(head, print_int);
    printf("\n");

    /* 反转 */
    generic_slist_reverse(&head);

    /* 反转后 */
    printf("反转后: ");
    slist_print(head, print_int);
    printf("\n");

    /* 清理资源 */
    slist_destroy(&head, NULL);

    return 0;
}