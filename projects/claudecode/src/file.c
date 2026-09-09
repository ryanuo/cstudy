#include <stdio.h>
#include <stdlib.h>

#include "../include/file.h"

char *file_read(const char *path)
{
    FILE *fp = fopen(path, "r");

    if (fp == NULL)
    {
        perror("fopen");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);

    long size = ftell(fp);

    rewind(fp);

    char *buf = malloc(size + 1);

    if (buf == NULL)
    {
        fclose(fp);
        return NULL;
    }

    size_t n = fread(
        buf,
        1,
        size,
        fp
    );

    buf[n] = '\0';

    fclose(fp);

    return buf;
}

int file_write(
    const char *path,
    const char *content
)
{
    FILE *fp = fopen(path, "w");

    if (fp == NULL)
    {
        perror("fopen");
        return -1;
    }

    fputs(content, fp);

    fclose(fp);

    return 0;
}