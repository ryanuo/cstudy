#include <stdio.h>

int main()
{
    int num, sum = 0;

    printf("请输入一个正整数: ");
    scanf("%d", &num);

    if (num <= 0)
    {
        printf("请输入一个正整数！\n");
        return 1;
    }

    for (int i = 1; i <= num; i++)
    {
        if (i % 2 == 0) // 如果是偶数，跳过
        {
            continue;
        }

        sum += i;
    }

    printf("从1到%d的整数之和是: %d\n", num, sum);
    return 0;
}