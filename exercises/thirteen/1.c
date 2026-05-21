#include <stdio.h>
#include <stdlib.h>
#include "test.h"

void menu()
{
    printf("====================\n");
    printf("1.        用户登录\n");
    printf("2.        用户注册\n");
    printf("3.        退出程序\n");
    printf("====================\n");
}

int main(int argc, char **argv)
{
    int val;

    printf("请选择以下菜单选项(1-3)\n");
    while (1)
    {
        menu();
        scanf("%d", &val);
        clear_buffer();

        switch (val)
        {
        case 1:
            login_user();
            break;

        case 2:
            sign_user();
            break;

        case 3:
            printf("退出程序!!\n");
            exit(0);
            break;

        default:
            printf("输入错误，请重新选择。\n");
            break;
        }
    }

    return 0;
}