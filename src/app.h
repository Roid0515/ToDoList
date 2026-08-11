#pragma once
#include <windows.h>
#include "database.h"
#include "paths.h"
#include "settings.h"

typedef struct AppState {
    HINSTANCE instance;
    HWND hwnd_main;
    HWND date_picker, time_picker, time_check, title_edit, desc_edit;
    HWND completed_check, priority_combo, add_button, update_button, delete_button, clear_button;
    HWND task_list, topmost_check, opacity_trackbar, opacity_label;
    Database database;
    AppPaths paths;
    AppSettings settings;
    int64_t selected_task_id;
} AppState;

int app_run(HINSTANCE instance, int show_command);

