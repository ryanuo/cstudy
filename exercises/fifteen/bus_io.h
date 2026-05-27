#ifndef __BUS_IO_H__
#define __BUS_IO_H__

#include "bus.h"
#include "generic_list.h"

/*
 * I/O 模块
 * 负责文件的读写操作
 * 与业务逻辑解耦
 */

int bus_save_to_file(const char *filename, const generic_list_t *head);
int bus_load_from_file(const char *filename, generic_list_t **head);

#endif
