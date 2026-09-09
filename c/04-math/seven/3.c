#include <stdio.h>
#include <math.h>

int main()
{
    int point1[2] = {0};
    int point2[2] = {0};
    int getDistance(int[], int[]);

    printf("请输入第一个点的坐标 (x y)：");
    scanf("%d %d", &point1[0], &point1[1]);

    printf("请输入第二个点的坐标 (x y)：");
    scanf("%d %d", &point2[0], &point2[1]);

    int distance = getDistance(point1, point2);

    printf("两点之间的距离为：%d\n", distance);
    return 0;
}

int getDistance(int point1[], int point2[])
{
    int dx = point2[0] - point1[0];
    int dy = point2[1] - point1[1];

    return (int)sqrt(dx * dx + dy * dy);
}
