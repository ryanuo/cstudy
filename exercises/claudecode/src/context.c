#include <stdio.h>
#include <string.h>

#include "../include/context.h"

void context_init(Context *ctx)
{
    ctx->count = 0;
}

void context_add(
    Context *ctx,
    const char *msg
)
{
    if (ctx->count >= MAX_HISTORY)
    {
        for (int i = 1; i < MAX_HISTORY; i++)
        {
            strcpy(
                ctx->messages[i - 1],
                ctx->messages[i]
            );
        }

        ctx->count--;
    }

    strncpy(
        ctx->messages[ctx->count],
        msg,
        MAX_MESSAGE_LEN - 1
    );

    ctx->messages[ctx->count][MAX_MESSAGE_LEN - 1]
        = '\0';

    ctx->count++;
}

void context_print(
    const Context *ctx
)
{
    printf("\n==== History ====\n");

    for (int i = 0; i < ctx->count; i++)
    {
        printf(
            "[%d] %s\n",
            i + 1,
            ctx->messages[i]
        );
    }

    printf("=================\n");
}