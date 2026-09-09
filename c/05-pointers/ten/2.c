#include <stdio.h>

int str_len(char *p)
{
    int n = 0;

    for (; *p != '\0'; p++)
    {
        n++;
    }

    return n;
}

int str_upp_len(char *p)
{
    int n = 0;
    for (; *p != '\0'; p++)
    {
        if (*p >= 'A' && *p <= 'Z')
        {
            n++;
        }
    }

    return n;
}

int str_num_len(char *p)
{
    int n = 0;
    for (; *p != '\0'; p++)
    {
        if (*p >= '0' && *p <= '9')
        {
            n++;
        }
    }

    return n;
}

int main()
{
    char a[] = "Hello World9991";

    printf("当前a的字符串长度为：%d\n", str_len(a));
    printf("当前a的数字个数为：%d\n", str_num_len(a));
    printf("当前a的大写字母个数为：%d\n", str_upp_len(a));
    return 0;
}