#include <stdio.h>

int *max_pointer(int size, int (*a)[size])
{
    int *max = &a[0][0];
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (a[i][j] > *max)
            {
                max = &a[i][j];
            }
        }
    }

    printf("max = %d\n", *max);

    return max;
}

int main()
{
    int a[][3] = {
        {0, 0, 0},
        {4, 5, 6},
        {7, 8, 0}};

    printf("max_address = %p\n", max_pointer(3, a));

    return 0;
}