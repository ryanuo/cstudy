#include <stdio.h>

int main()
{
    int i, j;
    int flag = 5;
    int mid = flag / 2;
    for (i = 0; i < flag; i++)
    {
        // 分成两部分计算 上半部分0,1 下半部分：2,3,4
        int val = (i < mid) ? i : flag - 1 - i;
        int stars = val + 1; // 星星的数量
        for (j = 0; j < stars; j++)
        {
            printf("* ");
        }
        printf("\n");
    }
    return 0;
}
