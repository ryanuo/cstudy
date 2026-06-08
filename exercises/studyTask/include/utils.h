#ifndef __UTILS_H__
#define __UTILS_H__

#include <time.h>

void timestamp_to_string(time_t ts,
                         char *buf,
                         int size);

time_t string_to_timestamp(const char *str);

const char *time_to_str(time_t t);

void clear_buffer();

void trim_newline(char *s);

int input_int(const char *prompt,
              int min,
              int max);

int input_yes_no(const char *prompt);
#endif