#include <stdio.h>

int upp_words_counts(char words[], int len)
{
    int n = 0;

    for (int i = 0; i < len; i++)
    {
        if (words[i] <= 90)
        {
            n++;
        }
    }

    return n;
}

int main()
{
    char strs[100];

    printf("请输入一个字符串：");
    scanf("%s", strs);

    int len = sizeof(strs) / sizeof(strs[0]);

    printf("字符串中大写字母的个数为：%d\n", upp_words_counts(strs, len));

    return 0;
}