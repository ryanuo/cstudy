#include <stdio.h>

#include "slist.h"

int main(int argc, char **argv)
{
    slist_t *head = NULL;

    for (int i = 0; i < 10; i++)
    {
        // slist_addhead
        slist_addtail(&head, i);
    }

    slist_showall(head);
    return 0;
}