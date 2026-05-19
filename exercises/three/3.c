#include <stdio.h>

int main(void)
{
    double height, weight, bmi;

    printf("请输入身高（m）和体重（kg）：");
    scanf("%lf%lf", &height, &weight);

    bmi = weight / (height * height);

    printf("身高：%.2lf m，体重：%.2lf kg，BMI：%.2lf\n", height, weight, bmi);
    return 0;
}