#include <stdio.h>
#include <stdlib.h>

void remove_element(int *a, int *len, int target)
{
    if (target < 0 || target >= *len)
    {
        printf("下标越界，删除失败\n");
        return;
    }

    for (int i = target; i < *len - 1; i++)
    {
        a[i] = a[i + 1];
    }

    (*len)--;
}

int main(int argc, char **argv)
{
    int initial_size = 5;
    int len = initial_size;

    int *a = (int *)calloc(initial_size, sizeof(int));
    if (a == NULL)
    {
        printf("内存分配失败\n");
        return -1;
    }

    printf("原数组为：\n");
    for (int i = 0; i < len; i++)
    {
        a[i] = i;
        printf("%d ", a[i]);
    }
    printf("\n");

    // 删除第3个元素（下标为2）
    remove_element(a, &len, 2);

    int *temp = (int *)realloc(a, len * sizeof(int));
    if (temp == NULL)
    {
        printf("内存重分配失败，释放原内存\n");
        free(a);
        return -1;
    }
    a = temp;

    printf("删除第3个元素后的数组为：\n");
    for (int i = 0; i < len; i++)
    {
        printf("%d ", a[i]);
    }
    printf("\n");

    free(a);
    a = NULL;

    return 0;
}