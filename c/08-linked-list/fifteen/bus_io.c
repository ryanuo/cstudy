#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bus_io.h"
#include "generic_list.h"

int bus_save_to_file(const char *filename, const generic_list_t *head)
{
    if (filename == NULL)
        return -1;

    FILE *file = fopen(filename, "w+");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }

    const generic_list_t *p = head;
    while (p != NULL)
    {
        bus_t *bus = (bus_t *)p->data;
        fprintf(file, "%s,%s,%s,%.2f\n",
                bus->line_name,
                bus->start_stat,
                bus->end_stat,
                bus->mileage);
        p = p->next;
    }

    fclose(file);
    return 0;
}

int bus_load_from_file(const char *filename, generic_list_t **head)
{
    if (filename == NULL || head == NULL)
        return -1;

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return -1;
    }

    bus_t temp;
    while (fscanf(file,
                  "%[^,],%[^,],%[^,],%f\n",
                  temp.line_name,
                  temp.start_stat,
                  temp.end_stat,
                  &temp.mileage) == 4)
    {
        slist_add_head(head, &temp, sizeof(bus_t));
    }

    fclose(file);
    return 0;
}
