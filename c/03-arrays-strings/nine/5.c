#include <stdio.h>

float ava_float_data(float *p, int size)
{
    float sum = 0;
    int len = size;

    for (; size > 0; size--)
    {
        sum += *p++;
    }

    return sum / len;
}

int main()
{
    float data[] = {1.1, 2.2, 3.3, 4.4, 5.5};

    printf("平均值为%.2f\n", ava_float_data(data, 5));
    return 0;
}