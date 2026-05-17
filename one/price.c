#include <stdio.h>

int main()
{
    double area, price;

    printf("请输入房子的面积: ");

    if (scanf("%lf", &area) != 1 || area <= 0)
    {
        printf("输入格式错误！\n");
        return 1;
    }

    price = area * 15000;

    printf("房子的价格是: %.2lf\n", price);
    return 0;
}
