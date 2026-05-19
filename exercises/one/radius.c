#include <stdio.h>

int main()
{
    double radius, area;

    printf("请输入圆的半径: ");
    if (scanf("%lf", &radius) == 1)
    {
        area = 3.14159 * radius * radius;
        printf("圆的面积是: %.2lf\n", area);
    }
    else
    {
        printf("输入格式错误！\n");
    }

    return 0;
}
