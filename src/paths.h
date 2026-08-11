#pragma once
#include <windows.h>

typedef struct AppPaths {
    wchar_t root[MAX_PATH];
    wchar_t data[MAX_PATH];
    wchar_t config[MAX_PATH];
    wchar_t logs[MAX_PATH];
    wchar_t database[MAX_PATH];
    wchar_t settings[MAX_PATH];
    wchar_t log[MAX_PATH];
} AppPaths;

BOOL paths_initialize(AppPaths *paths);

