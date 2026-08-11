#pragma once
#include "task.h"
#include "sqlite3.h"
#include <windows.h>

typedef struct Database { sqlite3 *handle; } Database;

BOOL database_open(Database *db, const wchar_t *path, wchar_t *error, size_t error_cap);
void database_close(Database *db);
BOOL database_insert(Database *db, const Task *task, wchar_t *error, size_t error_cap);
BOOL database_update(Database *db, const Task *task, wchar_t *error, size_t error_cap);
BOOL database_delete(Database *db, int64_t id, wchar_t *error, size_t error_cap);
BOOL database_get(Database *db, int64_t id, Task *task, wchar_t *error, size_t error_cap);
BOOL database_list(Database *db, TaskList *list, wchar_t *error, size_t error_cap);

