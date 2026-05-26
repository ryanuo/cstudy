#include <stdio.h>

#include "bus.h"

int main(int argc, char **argv)
{
    dlist_t *head = NULL;

    bus_t bus_line_data[] = {
        {"1路", "火车站", "大学城", 12.5},
        {"2路", "汽车站", "市政府", 8.3},
        {"3路", "人民广场", "高铁站", 15.7},
        {"5路", "东门", "西门", 6.8},
        {"8路", "体育馆", "科技园", 18.2},
        {"10路", "南站", "北站", 20.0},
        {"12路", "幸福小区", "中心医院", 9.6},
        {"15路", "图书馆", "机场", 25.4}};

    for (int i = 0; i < 8; i++)
    {
        dlist_addhead(&head, bus_line_data[i]);
    }
    dlist_showall(head);

    // 保存
    save_busdata((base_node_t*) head);

    // slist_t *head1 = NULL;

    // read_del_sdata(&head1);

    // save_busdata((base_node_t*) head1);

    return 0;
}