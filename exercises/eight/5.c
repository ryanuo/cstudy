#include <stdio.h>
#include <string.h>

char *reverse_data(char data[], int size)
{
    for (int i = 0; i < size / 2; i++)
    {
        char temp = data[i];
        data[i] = data[size - i - 1];
        data[size - i - 1] = temp;
    }

    return data;
}

// 递归翻转
void reverse_data_recursive(char data[], int size)
{
    if (size == 2)
        return;

    char temp = data[0];
    data[0] = data[size - 1];
    data[size - 1] = temp;
    reverse_data_recursive(data + 1, size - 2);
}

int main()
{
    char str[] = "hello world";
    char str2[] = "hello world";

    printf("翻转前: %s\n", str);
    reverse_data_recursive(str, strlen(str));
    printf("翻转后: %s\n", str);

    printf("翻转前: %s\n", str2);
    reverse_data(str2, strlen(str2));
    printf("翻转后: %s\n", str2);

    return 0;
}