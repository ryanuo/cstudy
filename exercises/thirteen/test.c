#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

void clear_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int login_user()
{
    user_t input_user, exit_user;
    FILE *fp;

    printf("请输入你要登录的账号密码(账号 密码)，使用空格隔开:\n");
    scanf("%s %s", input_user.name, input_user.password);

    if ((fp = fopen("user.dat", "r")) == NULL)
    {
        perror("fopen");
        return error;
    }

    int flag = 0;
    while (fscanf(fp, "%s %s", exit_user.name, exit_user.password) != EOF)
    {
        if (strcmp(exit_user.name, input_user.name) == 0 && strcmp(exit_user.password, input_user.password) == 0)
        {
            flag = 1;
            break;
        }
    }

    fclose(fp);
    if (flag)
    {
        printf("登录成功！！\n");
        exit(0);
    }
    else
    {
        printf("登录失败：用户名或密码错误，请重试。\n");
    }

    return error;
}

int sign_user()
{
    user_t new_user, exit_user;
    FILE *fp;

    printf("请输入你要注册的账号密码(账号 密码)，使用空格隔开:\n");
    scanf("%s %s", new_user.name, new_user.password);

    if ((fp = fopen("user.dat", "a+")) == NULL)
    {
        printf("系统提示：用户文件不存在，请先注册。\n");
        perror("fopen");
        return error;
    }

    while (fscanf(fp, "%s %s", exit_user.name, exit_user.password) != EOF)
    {
        if (strcmp(new_user.name, exit_user.name) == 0)
        {
            fclose(fp);
            printf("当前用户已存在，请重新注册！\n");
            return error;
        }
    }

    if (fprintf(fp, "\n%s %s", new_user.name, new_user.password) > 0)
    {
        printf("注册成功！");
        fclose(fp);
        exit(0);
    }
    else
    {
        fclose(fp);
        printf("写入失败请检查！");
    }

    return success;
}
