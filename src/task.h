#pragma once
#include <stdint.h>
#include <wchar.h>

#define TASK_TITLE_CAP 201
#define TASK_DESC_CAP 2001

typedef struct Task {
    int64_t id;
    wchar_t task_date[11];
    wchar_t task_time[6];
    wchar_t title[TASK_TITLE_CAP];
    wchar_t description[TASK_DESC_CAP];
    int completed;
    int priority;
} Task;

typedef struct TaskList {
    Task *items;
    size_t count;
} TaskList;

void task_list_free(TaskList *list);

