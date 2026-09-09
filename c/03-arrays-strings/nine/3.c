#include <stdio.h>

int main()
{
    printf("整型的指针变量的大小为：%zu\n", sizeof(int *)); // 8
    printf("浮点型的指针变量的大小为：%zu\n", sizeof(float *)); // 8
    printf("字符型指针变量的大小为：%zu\n", sizeof(char *)); // 8

    return 0;
}