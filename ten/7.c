#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    int a = 1;

    int *p = (int *)memset(&a, 1, sizeof(int));

    printf("%b\n", a); // 输出二进制
    return 0;
}