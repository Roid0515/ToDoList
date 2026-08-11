#include "task.h"
#include <stdlib.h>

void task_list_free(TaskList *list) {
    if (!list) return;
    free(list->items);
    list->items = NULL;
    list->count = 0;
}

