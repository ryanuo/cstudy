#include <stdio.h>

int is_num(char c)
{
    return c >= '0' && c <= '9';
}

int sum_numbers(char *a)
{
    int sum = 0;
    int num = 0;
    int in_number = 0;

    for (int i = 0; a[i] != '\0'; i++)
    {
        if (is_num(a[i]))
        {
            // 如果当前是数字，前面暂存的num需要要向左移动一位，也就是*10
            num = num * 10 + (a[i] - '0');
            in_number = 1;
        }
        else
        {
            if (in_number)
            {
                sum += num;
                num = 0;
                in_number = 0;
            }
        }
    }

    if (in_number)
    {
        sum += num;
    }

    return sum;
}

int main(int argc, char **argv)
{
    char *a = "123abd23ee1";

    printf("字符串中所有数字之和为：%d\n", sum_numbers(a));

    return 0;
}