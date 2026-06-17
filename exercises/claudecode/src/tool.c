#include <stdio.h>
#include <stdlib.h>

#include "../include/tool.h"
#include "../include/file.h"
#include "../include/shell.h"

int tool_execute(
    ToolCall *call
)
{
    switch (call->type)
    {
        case TOOL_READ_FILE:
        {
            char *content =
                file_read(call->path);

            if (content == NULL)
            {
                printf(
                    "read failed\n"
                );

                return -1;
            }

            printf(
                "\n========== %s ==========\n",
                call->path
            );

            printf(
                "%s\n",
                content
            );

            printf(
                "========================\n"
            );

            free(content);

            break;
        }

        case TOOL_RUN_CMD:
        {
            printf(
                "\n$ %s\n\n",
                call->command
            );

            shell_exec(
                call->command
            );

            break;
        }

        default:
        {
            printf(
                "unknown command\n"
            );

            break;
        }
    }

    return 0;
}