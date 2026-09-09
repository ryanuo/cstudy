#include <stdio.h>

int main(void)
{
    int a, b, c;

    printf("请输入通话秒数：");
    scanf("%d", &a);

    b = a / 60; // 计算分钟数
    c = a % 60; // 计算剩余秒数

    printf("%d分钟%d秒\n", b, c);
    return 0;
}