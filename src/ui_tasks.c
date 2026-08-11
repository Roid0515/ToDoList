#include "ui_internal.h"
#include "log.h"
#include <strsafe.h>
#include <wctype.h>
#include <windowsx.h>

static void show_database_error(AppState *app, const wchar_t *message)
{
    log_write(message);
    MessageBoxW(
        app->hwnd_main,
        message,
        L"Personal ToDo",
        MB_ICONERROR);
}

static BOOL read_form(AppState *app, Task *task)
{
    SYSTEMTIME date;
    SYSTEMTIME time;
    wchar_t *title_start;

    ZeroMemory(task, sizeof(*task));
    task->id = app->selected_task_id;

    DateTime_GetSystemtime(app->date_picker, &date);
    StringCchPrintfW(
        task->task_date,
        11,
        L"%04u-%02u-%02u",
        date.wYear,
        date.wMonth,
        date.wDay);

    if (Button_GetCheck(app->time_check) == BST_CHECKED) {
        DateTime_GetSystemtime(app->time_picker, &time);
        StringCchPrintfW(
            task->task_time,
            6,
            L"%02u:%02u",
            time.wHour,
            time.wMinute);
    }

    GetWindowTextW(app->title_edit, task->title, TASK_TITLE_CAP);
    GetWindowTextW(app->desc_edit, task->description, TASK_DESC_CAP);

    title_start = task->title;
    while (*title_start && iswspace(*title_start)) {
        ++title_start;
    }

    if (!*title_start) {
        MessageBoxW(
            app->hwnd_main,
            L"제목을 입력해 주세요.",
            L"입력 확인",
            MB_ICONWARNING);
        SetFocus(app->title_edit);
        return FALSE;
    }

    task->completed = Button_GetCheck(app->completed_check) == BST_CHECKED;
    task->priority = ComboBox_GetCurSel(app->priority_combo) + 1;
    if (task->priority < 1 || task->priority > 4) {
        task->priority = 3;
    }
    return TRUE;
}

static const wchar_t *priority_name(int value)
{
    static const wchar_t *names[] = {
        L"긴급",
        L"높음",
        L"보통",
        L"낮음"
    };

    if (value < 1 || value > 4) {
        return L"보통";
    }
    return names[value - 1];
}

static BOOL selected_task_id(AppState *app, int64_t *task_id)
{
    int row = ListView_GetNextItem(app->task_list, -1, LVNI_SELECTED);
    LVITEMW item;

    if (row < 0) {
        return FALSE;
    }

    ZeroMemory(&item, sizeof(item));
    item.mask = LVIF_PARAM;
    item.iItem = row;

    if (!ListView_GetItem(app->task_list, &item)) {
        return FALSE;
    }

    *task_id = (int64_t)item.lParam;
    return TRUE;
}

void ui_tasks_refresh(AppState *app)
{
    TaskList list = {0};
    wchar_t error[512];
    size_t index;

    if (!database_list(&app->database, &list, error, 512)) {
        show_database_error(app, error);
        return;
    }

    SendMessageW(app->task_list, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(app->task_list);

    for (index = 0; index < list.count; ++index) {
        Task *task = &list.items[index];
        LVITEMW item;
        int row;

        ZeroMemory(&item, sizeof(item));
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = (int)index;
        item.pszText = (LPWSTR)priority_name(task->priority);
        item.lParam = (LPARAM)task->id;

        row = ListView_InsertItem(app->task_list, &item);
        ListView_SetItemText(app->task_list, row, 1, task->task_date);
        ListView_SetItemText(
            app->task_list,
            row,
            2,
            task->task_time[0] ? task->task_time : L"-");
        ListView_SetItemText(app->task_list, row, 3, task->title);
        ListView_SetItemText(
            app->task_list,
            row,
            4,
            task->completed ? L"완료" : L"진행");
    }

    SendMessageW(app->task_list, WM_SETREDRAW, TRUE, 0);
    ui_layout_reset_task_columns(app);
    InvalidateRect(app->task_list, NULL, TRUE);
    task_list_free(&list);
}

void ui_tasks_load_selected(AppState *app)
{
    int64_t task_id;
    Task task;
    wchar_t error[512];
    SYSTEMTIME date_time;

    if (!selected_task_id(app, &task_id)) {
        return;
    }

    if (!database_get(&app->database, task_id, &task, error, 512)) {
        show_database_error(app, error);
        return;
    }

    GetLocalTime(&date_time);
    swscanf_s(
        task.task_date,
        L"%hu-%hu-%hu",
        &date_time.wYear,
        &date_time.wMonth,
        &date_time.wDay);
    DateTime_SetSystemtime(app->date_picker, GDT_VALID, &date_time);

    if (task.task_time[0]) {
        swscanf_s(
            task.task_time,
            L"%hu:%hu",
            &date_time.wHour,
            &date_time.wMinute);
        Button_SetCheck(app->time_check, BST_CHECKED);
        EnableWindow(app->time_picker, TRUE);
        DateTime_SetSystemtime(app->time_picker, GDT_VALID, &date_time);
    } else {
        Button_SetCheck(app->time_check, BST_UNCHECKED);
        EnableWindow(app->time_picker, FALSE);
    }

    SetWindowTextW(app->title_edit, task.title);
    SetWindowTextW(app->desc_edit, task.description);
    Button_SetCheck(
        app->completed_check,
        task.completed ? BST_CHECKED : BST_UNCHECKED);
    ComboBox_SetCurSel(app->priority_combo, task.priority - 1);
    app->selected_task_id = task.id;
    EnableWindow(app->update_button, TRUE);
    EnableWindow(app->delete_button, TRUE);
    SetFocus(app->title_edit);
}

void ui_tasks_add(AppState *app)
{
    Task task;
    wchar_t error[512];

    if (!read_form(app, &task)) {
        return;
    }
    if (!database_insert(&app->database, &task, error, 512)) {
        show_database_error(app, error);
        return;
    }

    ui_controls_reset_form(app);
    ui_tasks_refresh(app);
}
void ui_tasks_update(AppState *app)
{
    Task task;
    wchar_t error[512];

    if (!app->selected_task_id) {
        MessageBoxW(
            app->hwnd_main,
            L"목록에서 수정할 작업을 더블 클릭해 주세요.",
            L"작업 선택",
            MB_ICONINFORMATION);
        return;
    }
    if (!read_form(app, &task)) {
        return;
    }
    if (!database_update(&app->database, &task, error, 512)) {
        show_database_error(app, error);
        return;
    }

    ui_controls_reset_form(app);
    ui_tasks_refresh(app);
}

void ui_tasks_delete(AppState *app)
{
    wchar_t error[512];

    if (!app->selected_task_id) {
        MessageBoxW(
            app->hwnd_main,
            L"목록에서 삭제할 작업을 더블 클릭해 주세요.",
            L"작업 선택",
            MB_ICONINFORMATION);
        return;
    }

    if (MessageBoxW(
            app->hwnd_main,
            L"선택한 작업을 삭제하시겠습니까?",
            L"삭제 확인",
            MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES) {
        return;
    }

    if (!database_delete(
            &app->database,
            app->selected_task_id,
            error,
            512)) {
        show_database_error(app, error);
        return;
    }

    ui_controls_reset_form(app);
    ui_tasks_refresh(app);
}
