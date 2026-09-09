#ifndef TOOL_H
#define TOOL_H

#define MAX_PATH_LEN     256
#define MAX_CMD_LEN      512
#define MAX_CONTENT_LEN  4096

typedef enum
{
    TOOL_NONE = 0,

    TOOL_READ_FILE,
    TOOL_WRITE_FILE,

    TOOL_RUN_CMD

} ToolType;

typedef struct
{
    ToolType type;

    char path[MAX_PATH_LEN];

    char command[MAX_CMD_LEN];

    char content[MAX_CONTENT_LEN];

} ToolCall;

int tool_execute(
    ToolCall *call
);

#endif