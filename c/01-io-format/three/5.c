#include <stdio.h>

int main(void)
{
    unsigned short a = 0, b;

    b = a - 1;

    printf("a = %hu, b = %hu\n", a, b); // 0 65535
    return 0;
}