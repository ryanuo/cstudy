#include <stdio.h>

int main()
{
    int year, month, day;

    printf("请输入年月日（用空格分隔）：");
    scanf("%d %d", &year, &month);

    if (month < 1 || month > 12)
    {
        printf("无效的月份！\n");
        return 1;
    }

    if (month == 2)
    {
        day = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0 ? 29 : 28;
    }
    else if (month == 4 || month == 6 || month == 9 || month == 11)
    {
        day = 30;
    }
    else
    {
        day = 31;
    }

    printf("%d年%d月有%d天。\n", year, month, day);

    return 0;
}