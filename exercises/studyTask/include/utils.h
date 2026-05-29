#ifndef __UTILS_H__
#define __UTILS_H__

#include <time.h>

void timestamp_to_string(time_t ts,
                         char *buf,
                         int size);

time_t string_to_timestamp(const char *str);

#endif