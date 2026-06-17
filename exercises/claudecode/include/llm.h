#ifndef LLM_H
#define LLM_H

#include "tool.h"

int llm_chat(
    const char *prompt,
    ToolCall *call
);

#endif