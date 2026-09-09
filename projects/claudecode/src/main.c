#include <stdio.h>
#include <stdlib.h>

#include "../include/agent.h"

int main(void)
{
    printf("=================================\n");
    printf("      Claude Code C Edition\n");
    printf("=================================\n");
    printf("输入 exit 或 quit 退出\n\n");

    if (agent_init() != 0)
    {
        fprintf(stderr, "agent init failed\n");
        return EXIT_FAILURE;
    }

    agent_run();

    agent_destroy();

    return EXIT_SUCCESS;
}