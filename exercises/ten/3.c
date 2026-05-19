#include <stdio.h>
#include <stdlib.h>

int str_len(char *p)
{
    return 1;
}

int str_upp_len(char *p)
{
    return *p >= 'A' && *p <= 'Z';
}

int str_num_len(char *p)
{
    return *p >= '0' && *p <= '9';
}

int str_(char *p, int (*q)(char *))
{
    int n = 0;
    for (; *p != '\0'; p++)
    {
        if (q(p))
        {
            n++;
        }
    }

    return n;
}

int main()
{
    char a[] = "Hello World9991";
    int n;

    while (1)
    {
        printf("=======================\n");
        printf("1.求长度                  2.大写字母个数\n");
        printf("3.数字的个数              4.退出\n");
        printf("=======================\n");
        printf("请选择: ");
        scanf("%d", &n);

        switch (n)
        {
        case 1:
            printf("字符串的长度: %d\n", str_(a, str_len));
            break;
        case 2:
            printf("大写字母的个数: %d\n", str_(a, str_upp_len));
            break;
        case 3:
            printf("数字的个数: %d\n", str_(a, str_num_len));
            break;
        case 4:
            printf("退出程序\n");
            exit(0);
            break;
        }
    }

    return 0;
}