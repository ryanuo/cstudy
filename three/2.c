#include <stdio.h>
#define INCH 2.54

int main()
{
    double cm, inch;

    printf("请输入英寸数：");
    scanf("%lf", &cm);

    inch = cm / INCH;

    printf("%.2lf 厘米 = %.2lf 英寸\n", cm, inch);

    return 0;
}