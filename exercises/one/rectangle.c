#include <stdio.h>

int main()
{
    double width, height, area;

    printf("请输入矩形的长和宽: ");
    scanf("%lf %lf", &width, &height);

    area = width * height;

    printf("矩形的面积是: %.2lf\n", area);
}
