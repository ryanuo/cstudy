#include <stdio.h>

int charToDigit(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else
    {
        return -1;
    }
}

int main()
{
    char c;

    printf("请输入一个字符：");
    scanf("%c", &c);

    int digit = charToDigit(c);
    if (digit != -1)
    {
        printf("字符 '%c' 对应的数字是 %d\n", c, digit);
    }
    else
    {
        printf("字符 '%c' 不是一个数字\n", c);
    }

    return 0;
}