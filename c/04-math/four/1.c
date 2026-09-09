#include <stdio.h>

int main(void)
{
    double height, weight, bmi;

    printf("请输入您的身高(m)和体重(kg)：");
    scanf("%lf%lf", &height, &weight);

    bmi = weight / (height * height);

    if (bmi < 18.5)
    {
        puts("偏瘦，注意加强营养");
    }
    else if (18.5 <= bmi && bmi < 23.9)
    {
        puts("体重指数良好，注意保持");
    }
    else
    {
        puts("你有点偏胖，注意锻炼");
    }
    
    return 0;
}