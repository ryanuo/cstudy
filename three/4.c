#include <stdio.h>

int main(void)
{
    double a, result;

    printf("请输入一个摄氏度：");
    scanf("%lf", &a);

    result = a * 1.8 + 32;

    printf("%.2lf 摄氏度 = %.2lf 华氏度\n", a, result);
    return 0;
}