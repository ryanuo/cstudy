#ifndef AGENT_H
#define AGENT_H

#include "../include/context.h"

static Context g_ctx;

int agent_init(void);

void agent_run(void);

void agent_destroy(void);

#endif