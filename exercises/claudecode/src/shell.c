#include <stdio.h>

#include "../include/shell.h"

int shell_exec(
    const char *cmd
)
{
    FILE *fp = popen(
        cmd,
        "r"
    );

    if (fp == NULL)
    {
        perror("popen");
        return -1;
    }

    char buf[1024];

    while (
        fgets(
            buf,
            sizeof(buf),
            fp
        ) != NULL
    )
    {
        printf("%s", buf);
    }

    int status = pclose(fp);

    return status;
}