#include <stdio.h>

int main()
{
    double ball_volume(int);
    int radius;

    printf("请输入球的半径：");
    scanf("%d", &radius);

    printf("球的体积为：%.2lf\n", ball_volume(radius));
    return 0;
}

double ball_volume(int radius)
{
    return (4.0 / 3.0) * 3.14159 * radius * radius * radius;
}
