#include "utils.h"

#include <stdio.h>
#include <string.h>

/*
    时间戳 -> 字符串
*/
void timestamp_to_string(time_t ts,
                         char *buf,
                         int size)
{
    struct tm info;

#ifdef _WIN32
    // Windows
    localtime_s(&info, &ts);
#else
    // Linux/macOS
    localtime_r(&ts, &info);
#endif

    strftime(buf,
             size,
             "%Y-%m-%d %H:%M:%S",
             &info);
}

/*
    字符串 -> 时间戳
*/
time_t string_to_timestamp(const char *str)
{
    struct tm t = {0};

    sscanf(str,
           "%d-%d-%d %d:%d:%d",
           &t.tm_year,
           &t.tm_mon,
           &t.tm_mday,
           &t.tm_hour,
           &t.tm_min,
           &t.tm_sec);

    t.tm_year -= 1900;
    t.tm_mon -= 1;

    return mktime(&t);
}