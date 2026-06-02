#ifndef __MENU_H__
#define __MENU_H__

#include "common.h"

void welcome();

void menu();

void menu_loop(task_t **p, int (*callback)(task_t *task));
#endif