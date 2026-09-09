#include <stdio.h>

int main()
{
    int num, sqrt_sums(int);

    printf("请输入一个整数：");
    scanf("%d", &num);

    int result = sqrt_sums(num);

    printf("数字 %d 的每位数字的平方和为：%d\n", num, result);
    return 0;
}

int sqrt_sums(int num)
{
    int sum = 0;

    while (num > 0)
    {
        int digit = num % 10;
        sum += digit * digit;
        num /= 10;
    }
    return sum;
}