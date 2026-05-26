#include <stdio.h>
#include <stdlib.h>

typedef struct SListNode
{
    int data;
    struct SListNode *next;
} slist_t;

int slist_addhead(slist_t **head, int data)
{
    slist_t *pnew = (slist_t *)malloc(sizeof(slist_t));

    if (pnew == NULL)
        return -1;

    pnew->data = data;
    pnew->next = *head;

    *head = pnew;

    return 0;
}

int slist_showall(const slist_t *head)
{
    const slist_t *p = head;

    while (p)
    {
        printf("%d ", p->data);
        p = p->next;
    }

    printf("\n");
    return 0;
}

void slist_reverse(slist_t **head)
{
    if (head == NULL || *head == NULL || (*head)->next == NULL)
    {
        return;
    }

    slist_t *p = (*head)->next;

    (*head)->next = NULL;

    while (p)
    {
        slist_t *temp = p;

        p = p->next;

        temp->next = *head;

        *head = temp;
    }
}
int main(int argc, char **argv)
{
    slist_t *head = NULL;

    for (int i = 0; i < 10; i++)
    {
        slist_addhead(&head, i * 100);
    }

    slist_showall(head);

    slist_reverse(&head);

    slist_showall(head);
    return 0;
}