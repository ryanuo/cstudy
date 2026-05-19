#include <stdio.h>

int count_zero(int size, int (*a)[size])
{
    int count = 0;

    for (int i = 0; i < size; i++)
    {
        for (int j = i; j < size; j++)
        {
            if (*(*(a + i) + j) == 0)
            {
                count++;
            }
        }
    }

    return count;
}

int main()
{
    int a[][3] = {
        {0, 0, 0},
        {4, 5, 6},
        {7, 8, 0}};

    printf("0的个数是：%d\n", count_zero(3, a));
    return 0;
}