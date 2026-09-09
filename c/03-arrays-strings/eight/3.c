#include <stdio.h>

int str_cmp(char left[], char right[])
{
    int i = 0;

    while (left[i] == right[i])
    {
        if (left[i] == '\0')
            return 0;
        i++;
    }

    return left[i] - right[i];
}

int main()
{
    char a[100], b[100];

    printf("请输入（left right）两个字符串：");
    scanf("%s %s", a, b);

    printf("这两个字符串的ASCII值相差为：%d\n", str_cmp(a, b));

    return 0;
}