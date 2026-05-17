#include <stdio.h>

int main()
{
    const int STORE_DATE = 512;
    int seats[20][25];

    for (int row = 0; row < 20; row++)
    {
        for (int col = 0; col < 25; col++)
        {
            if ((row + 1) * (row + 1) + (col + 1) * (col + 1) == STORE_DATE)
            {
                printf("恭喜行 %d, 列 %d的影迷获得大礼包!\n", row + 1, col + 1);
            }
        }
    }
    return 0;
}