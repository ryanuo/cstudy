#include <stdio.h>

int main()
{
    int n;
    int matrix[n][n];
    int sum = 0;

    printf("请输入一个矩阵行列数（例如3表示3x3矩阵）：");
    scanf("%d", &n);

    printf("请输入%d行%d列的矩阵元素：\n", n, n);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("元素[%d][%d]：", i, j);
            scanf("%d", &matrix[i][j]);
        }
    }

    printf("\n您输入的完整矩阵是：\n");
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%d\t", matrix[i][j]);

            if (j <= i)
            {
                sum += matrix[i][j];
            }
        }
        printf("\n");
    }

    printf("下三角元素的和为：%d\n", sum);
    return 0;
}