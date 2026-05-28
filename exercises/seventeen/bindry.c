#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bindry.h"

int bindry_insert(binary_t **root, int data)
{
    binary_t *pnew = (binary_t *)malloc(sizeof(binary_t));

    if (pnew == NULL)
        return -1;

    pnew->data = data;
    pnew->left = pnew->right = NULL;

    if (*root == NULL)
    {
        *root = pnew;
        return 0;
    }

    binary_t *p = *root, *parent = NULL;
    while (p)
    {
        parent = p;
        if (memcmp(&p->data, &data, sizeof(int)) < 0)
            p = p->right;
        else
            p = p->left;
    }

    if (memcmp(&parent->data, &data, sizeof(int)) < 0)
        parent->right = pnew;
    else
        parent->left = pnew;

    return 0;
}

void bindry_pretree(const binary_t *root)
{
    if (root == NULL)
        return;

    printf("%d ", root->data);
    bindry_pretree(root->left);
    bindry_pretree(root->right);
}

void bindry_midtree(const binary_t *root)
{
    if (root == NULL)
        return;

    bindry_midtree(root->left);
    printf("%d ", root->data);
    bindry_midtree(root->right);
}

void bindry_posttree(const binary_t *root)
{
    if (root == NULL)
        return;

    bindry_posttree(root->left);
    bindry_posttree(root->right);
    printf("%d ", root->data);
}