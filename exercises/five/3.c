#include <stdio.h>

int main()
{
    int n;
    double sum = 1.0;

    printf("请输入一个n正整数: ");
    scanf("%d", &n);

    for (int i = 2; i <= n; i++)
    {
        if (i % 2 == 0)
        {
            sum += 1.0 / i;
        }
        else
        {
            sum -= 1.0 / i;
        }
    }

    printf("当n=%d时，sum的值为: %.4lf\n", n, sum);

    return 0;
}