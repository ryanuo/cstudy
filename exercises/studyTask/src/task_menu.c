#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "task.h"
#include "utils.h"

void welcome(void)
{
    system("clear");
    system("echo '\033[?25l'");

    printf("\n\n");

    printf("\t┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
    printf("\t┃                                            ┃\n");
    printf("\t┃      \033[1;36m✨ 欢迎进入 StudyTask v1.0 ✨\033[0m         ┃\n");
    printf("\t┃                                            ┃\n");
    printf("\t┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\n");
    printf("\t┃                                            ┃\n");
    printf("\t┃      \033[1;33m你的专属控制台任务管理器\033[0m              ┃\n");
    printf("\t┃                                            ┃\n");
    printf("\t┃      🚀 \033[1;32m高效\033[0m · \033[1;34m简洁\033[0m · \033[1;35m易用\033[0m                 ┃\n");
    printf("\t┃                                            ┃\n");
    printf("\t┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n");

    printf("\n");
    printf("\t     \033[1;32m[ 系统已就绪，按回车键进入主菜单... ]\033[0m\n");

    getchar();

    system("echo '\033[?25h'");

    system("clear");
}

void menu()
{
    printf("\n");
    printf("\033[1;36m====== StudyTask 任务管理系统 ======\033[0m\n");
    printf("\033[32m 1.\033[0m 添加任务           \033[32m2.\033[0m 删除任务\n");
    printf("\033[32m 3.\033[0m 修改任务          \033[32m 4.\033[0m 查看所有任务\n");
    printf("\033[32m 5.\033[0m 查找任务          \033[32m 6.\033[0m 标记完成\n");
    printf("\033[32m 7.\033[0m 查看已完成任务    \033[32m 8.\033[0m 撤销操作\n");
    printf("\033[32m 9.\033[0m 任务统计\n");
    printf("\033[1;36m====================================\033[0m\n");
    printf("\033[1;31m 0. 退出系统\033[0m\n");
    printf("\033[33m--------------------------------------\033[0m\n");
    printf(" 请选择操作: ");
}

void menu_loop(task_t **task, int (*callback)(task_t *t))
{
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

        clear_buffer();

        switch (selected_value)
        {
        case 1:
            task_add(task);
            break;
        case 5:
            task_search(*task);
            break;
        case 7:
            task_list_completed(*task);
            break;
        case 0:
            callback(*task);
            system("clear");
            printf("\n\n\n\n学习任务管理系统已退出！\n");
            exit(0);
            break;

        default:
            printf("无效菜单项！\n");
            break;
        }
    }
}