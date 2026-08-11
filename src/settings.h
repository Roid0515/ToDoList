#pragma once
#include <windows.h>

typedef struct AppSettings {
    int x, y, width, height;
    int opacity;
    int always_on_top;
} AppSettings;

void settings_defaults(AppSettings *settings);
void settings_load(const wchar_t *path, AppSettings *settings);
BOOL settings_save(const wchar_t *path, const AppSettings *settings);

