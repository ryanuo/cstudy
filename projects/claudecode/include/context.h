#ifndef CONTEXT_H
#define CONTEXT_H

#define MAX_HISTORY 50
#define MAX_MESSAGE_LEN 4096

typedef struct
{
    char messages[MAX_HISTORY][MAX_MESSAGE_LEN];

    int count;

} Context;

void context_init(Context *ctx);

void context_add(
    Context *ctx,
    const char *msg
);

void context_print(
    const Context *ctx
);

#endif