#include <stdio.h>
#define VAL 0.00001

int main()
{
    float a = 32.22;

    if (a - 32.22 < VAL)
    {
        printf("a is equal to 32.22\n");
    }
    else
    {
        printf("a is not equal to 32.22\n");
    }

    return 0;
}