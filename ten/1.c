#include <stdio.h>
#include <stdlib.h>

// 6) p为返回整形函数值的函数
int fp(int x)
{
    return 0;
}

// 7) p为返回一个指针的函数，该指针指向整形数据
int *p()
{
    int *a = (int *)malloc(sizeof(int));

    if (a != NULL)
    {
        return a;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    // 1) 定义整形变量i
    int i = 0;

    // 2) p为指向整形变量的指针变量
    int *p;

    // 3) 定义整形一维数组p,它有n个整形元素
    int p[5];

    // 4) 定义一维指针数组p,它有n个指向整形变量的指针元素
    int *p[9];

    // 5) 定义p为指向（含有n个整形元素的一维数组）的指针变量
    int (*p)[9];

    // 8) p为指向函数的指针变量，该函数返回一个整形值
    int (*p)(int) = fp;

    // 9) p是一个指向整形指针变量的指针变量
    int p7 = 0;
    int *p8 = &p7;
    int **p = &p8;

    return 0;
}