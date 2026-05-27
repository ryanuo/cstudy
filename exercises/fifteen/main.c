#include <stdio.h>
#include "bus_service.h"
#include "generic_list.h"

int main(int argc, char **argv)
{
    /* 初始化系统 */
    bus_system_t *system = bus_system_init();
    if (system == NULL)
    {
        printf("系统初始化失败\n");
        return -1;
    }

    /* 初始化数据 */
    bus_t bus_lines[] = {
        {"1路", "火车站", "大学城", 12.5},
        {"2路", "汽车站", "市政府", 8.3},
        {"3路", "人民广场", "高铁站", 15.7},
        {"5路", "东门", "西门", 6.8},
        {"8路", "体育馆", "科技园", 18.2},
        {"10路", "南站", "北站", 20.0},
        {"12路", "幸福小区", "中心医院", 9.6},
        {"15路", "图书馆", "机场", 25.4}
    };

    /* 添加到双向链表 */
    printf("=== 添加所有线路到双向链表 ===\n");
    for (int i = 0; i < 8; i++)
    {
        bus_add_to_dlist(system, &bus_lines[i]);
    }

    /* 显示所有线路 */
    printf("\n=== 双向链表中的所有线路 ===\n");
    bus_show_all(system->dlist);

    /* 保存到文件 */
    printf("\n=== 保存数据到文件 ===\n");
    if (bus_system_save(system, "bus.dat") == 0)
    {
        printf("保存成功\n");
    }

    /* 从文件加载数据并处理 */
    printf("\n=== 从文件加载并处理数据 ===\n");
    if (bus_system_load_and_process(system, "bus.dat") == 0)
    {
        printf("加载成功\n");
    }

    /* 显示删除最小里程后的单向链表 */
    printf("\n=== 删除最小里程后的单向链表 ===\n");
    bus_show_all(system->slist);

    /* 再次保存 */
    printf("\n=== 保存处理后的数据 ===\n");
    if (bus_system_save(system, "bus.dat") == 0)
    {
        printf("保存成功\n");
    }

    /* 清理资源 */
    bus_system_destroy(system);

    return 0;
}