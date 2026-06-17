#include <stdio.h>
#include <string.h>

#include "../include/llm.h"

int llm_chat(
    const char *prompt,
    ToolCall *call
)
{
    if (strncmp(prompt, "read ", 5) == 0)
    {
        call->type = TOOL_READ_FILE;

        strcpy(
            call->path,
            prompt + 5
        );

        return 0;
    }

    if (strncmp(prompt, "run ", 4) == 0)
    {
        call->type = TOOL_RUN_CMD;

        strcpy(
            call->command,
            prompt + 4
        );

        return 0;
    }

    call->type = TOOL_NONE;

    return 0;
}