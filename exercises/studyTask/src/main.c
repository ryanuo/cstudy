#include <stdio.h>
#include <stdlib.h>

#include "menu.h"

int main()
{
    welcome();

    int selected_value;
    while (1)
    {
        menu();

        scanf("%d", &selected_value);
        if (selected_value < 0 || selected_value > 9)
        {
            printf("您当前输入的格式有误需要重新选择，范围<0-9>\n");
            continue;
        }

        switch (selected_value)
        {
        case 0:
            system("clear");
            printf("\n\n\n学习任务管理系统已退出！\n");
            exit(0);
            break;

        default:
            break;
        }
    }
    return 0;
}