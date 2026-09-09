#include <stdio.h>

int main()
{
    int input;
    int ints[] = {4, 0, 1, 8, 2, 6, 9, 3, 5, 7};
    int length = sizeof(ints) / sizeof(ints[0]);

    printf("请输入一个整数，将输出所在位置（0-9）：");
    scanf("%d", &input);

    for (int i = 0; i < length; i++)
    {
        if (ints[i] == input)
        {
            printf("该整数在数组中的位置是：%d\n", i);
            break;
        }
    }

    return 0;
}