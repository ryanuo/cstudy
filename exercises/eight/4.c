#include <stdio.h>
#include <stdlib.h>

double add_nums(double num1, double num2);
double sub_nums(double num1, double num2);
double mult_nums(double num1, double num2);
double div_nums(double num1, double num2);
void menu();

int main()
{
    int select_index;
    double num1, num2;

    while (1)
    {
        menu();
        scanf("%d", &select_index);
        getchar();

        switch (select_index)
        {
        case 1:
            printf("您选择的是加法，请输入两个数字（空格隔开）：");
            scanf("%lf %lf", &num1, &num2);

            printf("结果：%.2f + %.2f = %.2f\n", num1, num2, add_nums(num1, num2));
            break;
        case 2:
            printf("您选择的是减法，请输入两个数字（空格隔开）：");
            scanf("%lf %lf", &num1, &num2);
            printf("结果：%.2f - %.2f = %.2f\n", num1, num2, sub_nums(num1, num2));
            break;
        case 3:
            printf("您选择的是乘法，请输入两个数字（空格隔开）：");
            scanf("%lf %lf", &num1, &num2);
            printf("结果：%.2f * %.2f = %.2f\n", num1, num2, mult_nums(num1, num2));
            break;
        case 4:
            printf("您选择的是除法，请输入两个数字（空格隔开）：");
            scanf("%lf %lf", &num1, &num2);
            if (num2 == 0)
            {
                printf("错误：除数不能为 0！\n");
            }
            else
            {
                printf("结果：%.2f / %.2f = %.2f\n", num1, num2, div_nums(num1, num2));
            }
            break;
        case 0:
            printf("感谢使用，再见！\n");
            exit(0);
        default:
            printf("无效选项，请重新输入！\n");
        }
    }

    return 0;
}

void menu()
{
    printf("\n*******************************************\n");
    printf("       1、加法       2、减法    3、乘法 \n");
    printf("       4、除法       0、退出\n");
    printf("*******************************************\n");
    printf("请选择您需要的操作：");
}

double add_nums(double num1, double num2)
{
    return num1 + num2;
}

double sub_nums(double num1, double num2)
{
    return num1 - num2;
}

double mult_nums(double num1, double num2)
{
    return num1 * num2;
}

double div_nums(double num1, double num2)
{
    return num1 / num2;
}