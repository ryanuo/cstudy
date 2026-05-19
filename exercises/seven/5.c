#include <stdio.h>

int main()
{
    int x, calculate_x(int);

    printf("请输入一个整数：");
    scanf("%d", &x);

    int result = calculate_x(x);

    printf("计算结果为：%d\n", result);
    return 0;
}

int calculate_x(int x)
{
    if (x > 5)
    {
        return 4 * x + 7;
    }
    else
    {
        return -2 * x + 3;
    }
}