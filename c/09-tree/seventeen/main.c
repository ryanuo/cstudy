#include <stdio.h>
#include <stdlib.h>

#include "bindry.h"

int main(int argc, char **argv)
{
    binary_t *root = NULL;

    int a[] = {6, 2, 8, 4, 9, 5, 7, 1, 3};

    for (int i = 0; i < 9; i++)
    {
        bindry_insert(&root, a[i]);
    }
    bindry_pretree(root); // 6 2 1 4 3 5 8 7 9
    printf("\n");
    bindry_midtree(root); // 1 2 3 4 5 6 7 8 9
    printf("\n");
    bindry_posttree(root); // 1 3 5 4 2 7 9 8 6
    printf("\n");
    return 0;
}