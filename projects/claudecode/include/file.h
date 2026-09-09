#ifndef FILE_H
#define FILE_H

char *file_read(const char *path);

int file_write(
    const char *path,
    const char *content
);

#endif