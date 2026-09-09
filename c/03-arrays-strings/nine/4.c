#include <stdio.h>

int main()
{
    char a[] = "hello world";

    for (char *p = a; *p != '\0'; p++)
    {
        printf("%c ", *p);
    }
    printf("\n");

    return 0;
}