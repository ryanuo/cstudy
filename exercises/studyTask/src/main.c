#include <stdio.h>
#include <stdlib.h>

#include "task_menu.h"
#include "file.h"

int main()
{
    task_t *task = NULL;
    welcome();

    file_init();

    task_load(&task);

    menu_loop(&task);

    return 0;
}