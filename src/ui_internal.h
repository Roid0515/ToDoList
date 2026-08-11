#pragma once

#include "app.h"
#include <commctrl.h>

#define UI_APP_CLASS L"PersonalTodoMainWindow"
#define UI_WM_RESET_COLUMNS (WM_APP + 1)
#define UI_TASK_COLUMN_COUNT 5

typedef struct ListColumnDef {
    const wchar_t *name;
    int width;
} ListColumnDef;

extern const ListColumnDef g_task_columns[UI_TASK_COLUMN_COUNT];

BOOL ui_controls_create(AppState *app);
void ui_controls_reset_form(AppState *app);

void ui_layout_controls(AppState *app, int width, int height);
void ui_layout_reset_task_columns(AppState *app);

void ui_tasks_refresh(AppState *app);
void ui_tasks_load_selected(AppState *app);
void ui_tasks_add(AppState *app);
void ui_tasks_update(AppState *app);
void ui_tasks_delete(AppState *app);

void ui_settings_apply_opacity(AppState *app);
void ui_settings_apply_topmost(AppState *app);
void ui_settings_save_window(AppState *app);
