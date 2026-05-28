#ifndef __BINDRY__H
#define __BINDRY__H

typedef struct BinaryTree
{
    int data;
    struct BinaryTree *left;
    struct BinaryTree *right;
} binary_t;

int bindry_insert(binary_t **root, int data);
void bindry_pretree(const binary_t *root);
void bindry_midtree(const binary_t *root);
void bindry_posttree(const binary_t *root);

#endif