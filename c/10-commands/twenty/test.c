#include <stdio.h>
#include <time.h>

int main(void)
{
    time_t file_time = 0;      // 1970-01-01
    time_t now = time(NULL);   // 当前系统时间

    struct tm *tm1 = localtime(&file_time);

    char buf1[64];
    strftime(buf1, sizeof(buf1), "%F %T", tm1);

    printf("第一次: %s\n", buf1);

    // 覆盖 localtime 的静态缓冲区
    localtime(&now);

    char buf2[64];
    strftime(buf2, sizeof(buf2), "%F %T", tm1);

    printf("第二次: %s\n", buf2);

    return 0;
}