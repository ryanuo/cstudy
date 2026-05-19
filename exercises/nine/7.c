#include <stdio.h>

void transpose(int size, int (*p)[size])
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < i; j++)
        {
            int temp = *(*(p + i) + j);
            *(*(p + i) + j) = *(*(p + j) + i);
            *(*(p + j) + i) = temp;
        }
    }
}

int main()
{
    int n[][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}};

    transpose(4, n);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("%d ", *(*(n + i) + j));
        }
        printf("\n");
    }

    return 0;
}