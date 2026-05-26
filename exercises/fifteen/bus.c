
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bus.h"

int save_busdata(const base_node_t *head)
{
    const base_node_t *p = head;
    FILE *bus_file;

    if ((bus_file = fopen("bus.dat", "w+")) == NULL)
    {
        perror("fopen");
        return -1;
    }

    while (p)
    {
        fprintf(bus_file, "%s,%s,%s,%.2f\n",
                p->data.line_name,
                p->data.start_stat,
                p->data.end_stat,
                p->data.mileage);
        p = p->next;
    }

    fclose(bus_file);

        return 0;
}

int compare_bus(list_data_t a, list_data_t b)
{
    return strcmp(a.line_name, b.line_name) == 0 &&
           strcmp(a.start_stat, b.start_stat) == 0 &&
           strcmp(a.end_stat, b.end_stat) == 0 &&
           a.mileage == b.mileage;
}

int read_del_sdata(slist_t **head)
{
    FILE *filep;

    if ((filep = fopen("bus.dat", "r")) == NULL)
    {
        return -1;
    }

    bus_t temp;

    while (fscanf(filep,
                  "%[^,],%[^,],%[^,],%f\n",
                  temp.line_name,
                  temp.start_stat,
                  temp.end_stat,
                  &temp.mileage) == 4)
    {
        slist_addhead(head, temp);
    }

    fclose(filep);

    list_data_t *min = find_min_data(*head);

    if (min != NULL)
    {
        slist_delete(head, *min);
        printf("%s\n", min->line_name);
    }

    return 0;
}

bus_t *find_min_data(slist_t *head)
{
    if (head == NULL)
        return NULL;

    bus_t *min = &head->data;

    slist_t *p = head->next;

    while (p)
    {
        if (p->data.mileage < min->mileage)
        {
            min = &p->data;
        }

        p = p->next;
    }

    return min;
}
