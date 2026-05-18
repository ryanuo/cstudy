#include <stdio.h>

struct Date
{
    int year;
    int month;
    int day;
};

int current_month_days(int, int);
int find_total_day(struct Date date);
int main(int argc, char **argv)
{
    struct Date date = {2021, 12, 31};

    printf("Total days: %d\n", find_total_day(date));

    return 0;
}

int current_month_days(int month, int year)
{
    if (month <= 0 || month > 12)
    {
        printf("Invalid month\n");
        return -1;
    }

    if (month == 2)
    {
        return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0 ? 29 : 28;
    }
    else if (month == 4 || month == 6 || month == 9 || month == 11)
    {
        return 30;
    }
    else
    {
        return 31;
    }
}
int find_total_day(struct Date date)
{
    if (date.month == 1)
    {
        return date.day;
    }

    int sum = 0;

    for (int i = 1; i < date.month; i++)
    {
        sum += current_month_days(i, date.year);
    }

    return sum + date.day;
}