#include <stdio.h>

int find_str_index(const char *str, char target)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == target)
        {
            return i;
        }
    }

    return -1;
}

int main(int argc, char **argv)
{
    const char *p;
    p = "hello world";

    printf("%d\n", find_str_index(p, 'w'));
    return 0;
}