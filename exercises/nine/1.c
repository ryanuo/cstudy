#include <stdio.h>

int main()
{
    int a[] = {1, 2, 4, 5, 6};
    int n = 5;

    printf("反向输出数组：");
    for (int *p = a + n - 1; p >= a; p--)
    {
        printf("%d ", *p);
    }
    printf("\n");

    return 0;
}