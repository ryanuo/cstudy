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

void binary_delnode(binary_t **root, int data)
{
    if (root == NULL || *root == NULL)
        return;

    binary_t *parent = NULL;
    binary_t *del = *root;

    /* 查找删除节点 */
    while (del)
    {
        if (data > del->data)
        {
            parent = del;
            del = del->right;
        }
        else if (data < del->data)
        {
            parent = del;
            del = del->left;
        }
        else
        {
            break;
        }
    }

    /* 没找到 */
    if (del == NULL)
        return;

    /* 情况1：有两个孩子 */
    if (del->left && del->right)
    {
        binary_t *replace_parent = del;
        binary_t *replace = del->left;

        /* 找左子树最大值 */
        while (replace->right)
        {
            replace_parent = replace;
            replace = replace->right;
        }

        del->data = replace->data;

        /* 转为删除 replace */
        del = replace;
        parent = replace_parent;
    }

    /* 情况2：最多一个孩子 */
    binary_t *child = NULL;

    if (del->left)
        child = del->left;
    else
        child = del->right;

    /* 删除根节点 */
    if (parent == NULL)
    {
        *root = child;
    }
    else if (parent->left == del)
    {
        parent->left = child;
    }
    else
    {
        parent->right = child;
    }

    free(del);
}

void bindry_deltree(const binary_t **root, int data)
{
    binary_t *del = *root;
    binary_t *replace = NULL;
    binary_t *parent = NULL;

    while (del)
    {
        if (memcmp(&del->data, &data, sizeof(int)) < 0)
        {
            parent = del;
            del = del->right;
        }
        else if (memcmp(&del->data, &data, sizeof(int)) > 0)
        {
            parent = del;
            del = del->left;
        }
        else
        {
            if (del->left)
            {
                parent = del;
                replace = del->left;
                while (replace->right)
                {
                    parent = replace;
                    replace = replace->right;
                }
                del->data = replace->data;

                if (parent->right == replace)
                    parent->right = replace->left;
                else
                    parent->left = replace->left;
                free(replace);
            }
            else if (del->right)
            {
                parent = del;
                replace = del->right;
                while (replace->left)
                {
                    parent = replace;
                    replace = replace->left;
                }
                del->data = replace->data;

                if (parent->right == replace)
                    parent->right = replace->right;
                else
                    parent->left = replace->right;
                free(replace);
            }
            else
            {
                if (parent == NULL)
                {
                    free(parent);
                    return 0;
                }
                if (parent->left == del)
                    parent->left = NULL;
                else
                    parent->right = NULL;
                free(replace);
            }
        }
    }
}