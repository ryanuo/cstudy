#include <stdio.h>
#include <string.h>

#include "../include/agent.h"
#include "../include/tool.h"
#include "../include/llm.h"

#define INPUT_SIZE 2048

int agent_init(void)
{
    context_init(&g_ctx);

    return 0;
}

void agent_destroy(void)
{
}

void agent_run(void)
{
    char input[INPUT_SIZE];

    while (1)
    {
        printf("\n>> ");

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';
        context_add(
            &g_ctx,
            input);

        if (strcmp(input, "exit") == 0 ||
            strcmp(input, "quit") == 0 ||
            strcmp(input, "history") == 0)
        {
            context_print(&g_ctx);
            break;
        }

        ToolCall call;

        memset(&call, 0, sizeof(call));

        if (llm_chat(input, &call) != 0)
        {
            printf("llm error\n");
            continue;
        }

        tool_execute(&call);
    }
}