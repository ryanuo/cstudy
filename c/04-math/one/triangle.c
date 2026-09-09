#include <stdio.h>

int main(void)
{
    double width, height, length, perimeter;

    printf("请输入三角形的三条边，并且注意长度问题：");
    scanf("%lf %lf %lf", &width, &height, &length);

    if (width <= 0 || height <= 0 || length <= 0)
    {
        printf("输入的边长必须为正数！\n");
        return 1;
    }

    if (width + height <= length || width + length <= height || height + length <= width)
    {
        printf("输入的边长不能构成三角形！\n");
        return 1;
    }

    perimeter = width + height + length;

    printf("三角形的周长是: %.2lf\n", perimeter);
    return 0;
}
