#include <stdio.h>

int main()
{
    int a[] = {1, 2, 3, 4, 5};
    int *p = a;
    int sum = 0;

    for (int i = 0; i < 5; i++)
    {
        if (i % 2 != 0)
        {
            sum += *(p + i);
        }
    }

    printf("%d\n", sum);
    return 0;
}