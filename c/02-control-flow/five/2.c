#include <stdio.h>

int main()
{
    float total = 1, item_number;
    int n = 2;

    while (1)
    {
        item_number = 1.0 / (n * (n + 1));
        total += item_number;
        if (item_number < 0.00001)
        {
            break;
        }
        n++;
    }

    printf("n的值为%d, Total: %f\n", n - 1, total);
    return 0;
}