#include <stdio.h>
#include <stdbool.h>

bool is_prime(int n)
{
    if (n <= 1)
        return false;
    for (int i = 2; i <= n / 2; i++)
    {
        if (n % i == 0)
            return false;
    }
    return true;
}

int main()
{
    int num;

    for (int i = 100; i <= 200; i++)
    {
        if (is_prime(i))
        {
            printf("%d ", i);
        }
    }

    return 0;
}