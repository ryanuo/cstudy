#include <stdio.h>

int find_char_index(char a[], int len, char b)
{
    for (int i = 0; i < len; i++)
        if (a[i] == b)
            return i;
}

int main()
{
    char b, a[] = "hello world";
    int len = sizeof(a) / sizeof(a[0]);

    printf("请输入一个字符(hello world):");
    scanf("%c", &b);

    printf("该字符 %c 所在的位置在位置 %d\n", b, find_char_index(a, len, b));
    return 0;
}
