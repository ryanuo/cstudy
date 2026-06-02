#include "utils.h"

#include <stdio.h>
#include <string.h>

/*
    时间戳 -> 字符串
*/
void timestamp_to_string(time_t ts, char *buf, int size)
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

const char *time_to_str(time_t t)
{
    static char buf[64];
    timestamp_to_string(t, buf, sizeof(buf));
    return buf;
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

void clear_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void trim_newline(char *s)
{
    int len = strlen(s);
    while (len > 0 &&
           (s[len - 1] == '\n' || s[len - 1] == '\r'))
    {
        s[--len] = '\0';
    }
}