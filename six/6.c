#include <stdio.h>
#include <string.h>

int main()
{
    char str[100];
    printf("请输入一个整数: ");
    scanf("%s", str);

    int len = strlen(str);
    int isPalindrome = 1;

    for (int i = 0; i < len / 2; i++)
    {
        if (str[i] != str[len - 1 - i])
        {
            isPalindrome = 0;
            break;
        }
    }

    if (isPalindrome)
    {
        printf("是回文数\n");
    }
    else
    {
        printf("不是回文数\n");
    }

    return 0;
}