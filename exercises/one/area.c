#include <stdio.h>
#include <math.h>

int main()
{
    double a, b, c;
    double p, area;

    printf("请输入三角形的三条边长 (用空格隔开): ");
    scanf("%lf %lf %lf", &a, &b, &c);

    if (a + b > c && a + c > b && b + c > a)
    {

        p = (a + b + c) / 2.0;

        area = sqrt(p * (p - a) * (p - b) * (p - c));

        printf("三角形的面积是: %.2lf\n", area);
    }
    else
    {
        printf("错误：输入的边长 %.2lf, %.2lf, %.2lf 无法构成三角形。\n", a, b, c);
    }

    return 0;
}
