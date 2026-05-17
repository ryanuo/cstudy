#include <stdio.h>

int main()
{
    int a[10] = {85, 92, 78, 65, 88, 90, 55, 73, 80, 95};
    int length = sizeof(a) / sizeof(a[0]);

    for (int i = 0; i < length - 1; i++)
    {
        for (int j = length - 1; j > i; j--)
        {
            if (a[j] < a[j - 1])
            {
                int temp = a[j];
                a[j] = a[j - 1];
                a[j - 1] = temp;
            }
        }
    }

    // 从小到大输出
    printf("排序后的值：\n");
    for (int i = 0; i < length; i++)
    {
        printf("%d ", a[i]);
    }
    printf("\n");

    return 0;
}