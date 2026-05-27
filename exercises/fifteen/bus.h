#ifndef __BUS_H__
#define __BUS_H__

/*
 * Bus 数据结构定义
 * 仅定义数据结构，不包含业务逻辑
 */

typedef struct busLine
{
    char line_name[20];      /* 线路名称 */
    char start_stat[50];     /* 起点 */
    char end_stat[50];       /* 终点 */
    float mileage;           /* 里程 */
} bus_t;

#endif