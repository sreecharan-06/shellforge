#include <stdio.h>
#include <readline/history.h>
#include "../include/history.h"

void print_history(void)
{
    HIST_ENTRY **entries = history_list();

    if (entries != NULL)
    {
        for (int i = 0; entries[i] != NULL; i++)
        {
            printf("%d  %s\n", i + 1, entries[i]->line);
        }
    }
}
