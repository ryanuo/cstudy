#include <stdio.h>

int main()
{
    int height, age;

    printf("请输入你的身高和年龄: ");

    if (scanf("%d %d", &height, &age) == 2)
    {
        printf("我的年龄是%d,身高是%d\n", age, height);
    }
    else
    {
        printf("输入格式错误！\n");
    }

    return 0;
}
