/*
 * Legacy UI implementation preserved for the V2 refactoring history.
 * The active implementation is split across ui_main.c, ui_controls.c,
 * ui_layout.c, ui_tasks.c, and ui_settings.c.
 */
#if 0

#include "ui.h"
#include "resource.h"
#include "log.h"
#include <commctrl.h>
#include <windowsx.h>
#include <strsafe.h>
#include <wctype.h>

#define APP_CLASS L"PersonalTodoMainWindow"
#define CONTROL_MARGIN 12
#define WM_APP_RESET_COLUMNS (WM_APP + 1)

static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
static void resize_task_columns(AppState *app);

static HWND control(HWND parent, const wchar_t *klass, const wchar_t *text, DWORD style, int id) {
    HWND item = CreateWindowExW(0, klass, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 0, 0, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(NULL), NULL);
    SendMessageW(item, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    return item;
}

static void create_label(HWND parent, const wchar_t *text) { control(parent, L"STATIC", text, SS_LEFT, 0); }

static void apply_opacity(AppState *app) {
    BYTE alpha = (BYTE)((app->settings.opacity * 255 + 50) / 100);
    SetLayeredWindowAttributes(app->hwnd_main, 0, alpha, LWA_ALPHA);
    if (app->opacity_label) { wchar_t text[32]; StringCchPrintfW(text, 32, L"투명도 %d%%", app->settings.opacity); SetWindowTextW(app->opacity_label, text); }
}

static void apply_topmost(AppState *app) {
    SetWindowPos(app->hwnd_main, app->settings.always_on_top ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

static BOOL create_controls(AppState *app) {
    HWND child; LVCOLUMNW column; int widths[] = {64, 88, 55, 220, 58}; const wchar_t *names[] = {L"우선순위", L"날짜", L"시간", L"제목", L"상태"};
    create_label(app->hwnd_main, L"날짜");
    app->date_picker = control(app->hwnd_main, DATETIMEPICK_CLASSW, L"", DTS_SHORTDATEFORMAT | WS_TABSTOP, IDC_DATE_PICKER);
    app->time_check = control(app->hwnd_main, L"BUTTON", L"시간 지정", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_TIME_CHECK);
    app->time_picker = control(app->hwnd_main, DATETIMEPICK_CLASSW, L"", DTS_TIMEFORMAT | WS_TABSTOP, IDC_TIME_PICKER);
    SendMessageW(app->time_picker, DTM_SETFORMATW, 0, (LPARAM)L"HH:mm");
    create_label(app->hwnd_main, L"제목 *");
    app->title_edit = control(app->hwnd_main, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP, IDC_TITLE_EDIT);
    SendMessageW(app->title_edit, EM_SETLIMITTEXT, TASK_TITLE_CAP - 1, 0);
    create_label(app->hwnd_main, L"내용");
    app->desc_edit = control(app->hwnd_main, L"EDIT", L"", WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_TABSTOP, IDC_DESC_EDIT);
    SendMessageW(app->desc_edit, EM_SETLIMITTEXT, TASK_DESC_CAP - 1, 0);
    create_label(app->hwnd_main, L"우선순위");
    app->priority_combo = control(app->hwnd_main, L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_TABSTOP, IDC_PRIORITY_COMBO);
    SendMessageW(app->priority_combo, CB_ADDSTRING, 0, (LPARAM)L"긴급"); SendMessageW(app->priority_combo, CB_ADDSTRING, 0, (LPARAM)L"높음");
    SendMessageW(app->priority_combo, CB_ADDSTRING, 0, (LPARAM)L"보통"); SendMessageW(app->priority_combo, CB_ADDSTRING, 0, (LPARAM)L"낮음");
    app->completed_check = control(app->hwnd_main, L"BUTTON", L"완료", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_COMPLETED_CHECK);
    app->add_button = control(app->hwnd_main, L"BUTTON", L"등록", BS_PUSHBUTTON | WS_TABSTOP, IDC_BTN_ADD);
    app->update_button = control(app->hwnd_main, L"BUTTON", L"수정", BS_PUSHBUTTON | WS_TABSTOP, IDC_BTN_UPDATE);
    app->delete_button = control(app->hwnd_main, L"BUTTON", L"삭제", BS_PUSHBUTTON | WS_TABSTOP, IDC_BTN_DELETE);
    app->clear_button = control(app->hwnd_main, L"BUTTON", L"새로 입력", BS_PUSHBUTTON | WS_TABSTOP, IDC_BTN_CLEAR);
    app->task_list = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 0, 0, 0, 0, app->hwnd_main, (HMENU)(INT_PTR)IDC_TASK_LIST, app->instance, NULL);
    SendMessageW(app->task_list, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    ListView_SetExtendedListViewStyle(app->task_list, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);
    ZeroMemory(&column, sizeof(column)); column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    for (int i = 0; i < 5; ++i) { column.iSubItem = i; column.pszText = (LPWSTR)names[i]; column.cx = widths[i]; ListView_InsertColumn(app->task_list, i, &column); }
    app->topmost_check = control(app->hwnd_main, L"BUTTON", L"항상 위", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_TOPMOST_CHECK);
    app->opacity_label = control(app->hwnd_main, L"STATIC", L"투명도", SS_LEFT, 0);
    app->opacity_trackbar = control(app->hwnd_main, TRACKBAR_CLASSW, L"", TBS_HORZ | TBS_AUTOTICKS | WS_TABSTOP, IDC_OPACITY_TRACKBAR);
    SendMessageW(app->opacity_trackbar, TBM_SETRANGE, TRUE, MAKELPARAM(30, 100)); SendMessageW(app->opacity_trackbar, TBM_SETTICFREQ, 10, 0);
    child = GetWindow(app->hwnd_main, GW_CHILD); while (child) { SendMessageW(child, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE); child = GetWindow(child, GW_HWNDNEXT); }
    return app->date_picker && app->task_list && app->opacity_trackbar;
}

static void layout_controls(AppState *app, int width, int height) {
    HWND child = GetWindow(app->hwnd_main, GW_CHILD); HWND labels[4] = {0}; int label_count = 0;
    while (child) { wchar_t klass[16]; GetClassNameW(child, klass, 16); if (lstrcmpW(klass, L"Static") == 0 && child != app->opacity_label && label_count < 4) labels[label_count++] = child; child = GetWindow(child, GW_HWNDNEXT); }
    int left = CONTROL_MARGIN, content = width - CONTROL_MARGIN * 2, label_w = 72, field_x = left + label_w, field_w = content - label_w;
    if (label_count >= 4) {
        MoveWindow(labels[0], left, 14, label_w, 24, TRUE); MoveWindow(labels[1], left, 46, label_w, 24, TRUE);
        MoveWindow(labels[2], left, 78, label_w, 24, TRUE); MoveWindow(labels[3], left, 158, label_w, 24, TRUE);
    }
    MoveWindow(app->date_picker, field_x, 10, 130, 26, TRUE); MoveWindow(app->time_check, field_x + 140, 11, 88, 24, TRUE); MoveWindow(app->time_picker, field_x + 230, 10, 92, 26, TRUE);
    MoveWindow(app->title_edit, field_x, 43, field_w, 26, TRUE); MoveWindow(app->desc_edit, field_x, 75, field_w, 74, TRUE);
    MoveWindow(app->priority_combo, field_x, 155, 115, 200, TRUE); MoveWindow(app->completed_check, field_x + 130, 157, 70, 24, TRUE);
    int button_y = 191, button_w = (content - 18) / 4;
    MoveWindow(app->add_button, left, button_y, button_w, 30, TRUE); MoveWindow(app->update_button, left + button_w + 6, button_y, button_w, 30, TRUE);
    MoveWindow(app->delete_button, left + (button_w + 6) * 2, button_y, button_w, 30, TRUE); MoveWindow(app->clear_button, left + (button_w + 6) * 3, button_y, button_w, 30, TRUE);
    int list_y = 232, footer_y = height - 45;
    MoveWindow(app->task_list, left, list_y, content, footer_y - list_y - 8, TRUE);
    resize_task_columns(app);
    MoveWindow(app->topmost_check, left, footer_y + 8, 80, 26, TRUE); MoveWindow(app->opacity_label, left + 92, footer_y + 10, 92, 22, TRUE);
    MoveWindow(app->opacity_trackbar, left + 180, footer_y + 5, content - 180, 34, TRUE);
}

static void reset_form(AppState *app) {
    SYSTEMTIME now; GetLocalTime(&now);
    DateTime_SetSystemtime(app->date_picker, GDT_VALID, &now); DateTime_SetSystemtime(app->time_picker, GDT_VALID, &now);
    Button_SetCheck(app->time_check, BST_UNCHECKED); EnableWindow(app->time_picker, FALSE);
    SetWindowTextW(app->title_edit, L""); SetWindowTextW(app->desc_edit, L""); Button_SetCheck(app->completed_check, BST_UNCHECKED);
    ComboBox_SetCurSel(app->priority_combo, 2); app->selected_task_id = 0;
    EnableWindow(app->update_button, FALSE); EnableWindow(app->delete_button, FALSE); SetFocus(app->title_edit);
}

static void resize_task_columns(AppState *app) {
    const int priority_width = 68;
    const int date_width = 88;
    const int time_width = 55;
    const int status_width = 58;
    RECT client;
    int title_width;

    GetClientRect(app->task_list, &client);
    title_width = (client.right - client.left) - priority_width - date_width - time_width - status_width;

    if (title_width < 80) title_width = 80;
    ListView_SetColumnWidth(app->task_list, 0, priority_width);
    ListView_SetColumnWidth(app->task_list, 1, date_width);
    ListView_SetColumnWidth(app->task_list, 2, time_width);
    ListView_SetColumnWidth(app->task_list, 3, title_width);
    ListView_SetColumnWidth(app->task_list, 4, status_width);
}

static BOOL read_form(AppState *app, Task *task) {
    SYSTEMTIME date, time; wchar_t *start;
    ZeroMemory(task, sizeof(*task)); task->id = app->selected_task_id;
    DateTime_GetSystemtime(app->date_picker, &date); StringCchPrintfW(task->task_date, 11, L"%04u-%02u-%02u", date.wYear, date.wMonth, date.wDay);
    if (Button_GetCheck(app->time_check) == BST_CHECKED) { DateTime_GetSystemtime(app->time_picker, &time); StringCchPrintfW(task->task_time, 6, L"%02u:%02u", time.wHour, time.wMinute); }
    GetWindowTextW(app->title_edit, task->title, TASK_TITLE_CAP); GetWindowTextW(app->desc_edit, task->description, TASK_DESC_CAP);
    start = task->title; while (*start && iswspace(*start)) ++start;
    if (!*start) { MessageBoxW(app->hwnd_main, L"제목을 입력해 주세요.", L"입력 확인", MB_ICONWARNING); SetFocus(app->title_edit); return FALSE; }
    task->completed = Button_GetCheck(app->completed_check) == BST_CHECKED;
    task->priority = ComboBox_GetCurSel(app->priority_combo) + 1; if (task->priority < 1 || task->priority > 4) task->priority = 3;
    return TRUE;
}

static const wchar_t *priority_name(int value) { static const wchar_t *names[] = {L"긴급", L"높음", L"보통", L"낮음"}; return value >= 1 && value <= 4 ? names[value - 1] : L"보통"; }

static void refresh_list(AppState *app) {
    TaskList list = {0}; wchar_t error[512];
    if (!database_list(&app->database, &list, error, 512)) { log_write(error); MessageBoxW(app->hwnd_main, error, L"Personal ToDo", MB_ICONERROR); return; }
    SendMessageW(app->task_list, WM_SETREDRAW, FALSE, 0); ListView_DeleteAllItems(app->task_list);
    for (size_t i = 0; i < list.count; ++i) {
        Task *t = &list.items[i]; LVITEMW item; int row;
        ZeroMemory(&item, sizeof(item)); item.mask = LVIF_TEXT | LVIF_PARAM; item.iItem = (int)i; item.pszText = (LPWSTR)priority_name(t->priority); item.lParam = (LPARAM)t->id;
        row = ListView_InsertItem(app->task_list, &item); ListView_SetItemText(app->task_list, row, 1, t->task_date);
        ListView_SetItemText(app->task_list, row, 2, t->task_time[0] ? t->task_time : L"-"); ListView_SetItemText(app->task_list, row, 3, t->title);
        ListView_SetItemText(app->task_list, row, 4, t->completed ? L"완료" : L"진행");
    }
    SendMessageW(app->task_list, WM_SETREDRAW, TRUE, 0);
    resize_task_columns(app);
    InvalidateRect(app->task_list, NULL, TRUE);
    task_list_free(&list);
}

static void load_selected(AppState *app) {
    int row = ListView_GetNextItem(app->task_list, -1, LVNI_SELECTED); LVITEMW item; Task task; wchar_t error[512]; SYSTEMTIME st;
    if (row < 0) return;
    ZeroMemory(&item, sizeof(item)); item.mask = LVIF_PARAM; item.iItem = row; if (!ListView_GetItem(app->task_list, &item)) return;
    if (!database_get(&app->database, (int64_t)item.lParam, &task, error, 512)) { log_write(error); MessageBoxW(app->hwnd_main, error, L"Personal ToDo", MB_ICONERROR); return; }
    GetLocalTime(&st); swscanf_s(task.task_date, L"%hu-%hu-%hu", &st.wYear, &st.wMonth, &st.wDay); DateTime_SetSystemtime(app->date_picker, GDT_VALID, &st);
    if (task.task_time[0]) { swscanf_s(task.task_time, L"%hu:%hu", &st.wHour, &st.wMinute); Button_SetCheck(app->time_check, BST_CHECKED); EnableWindow(app->time_picker, TRUE); DateTime_SetSystemtime(app->time_picker, GDT_VALID, &st); }
    else { Button_SetCheck(app->time_check, BST_UNCHECKED); EnableWindow(app->time_picker, FALSE); }
    SetWindowTextW(app->title_edit, task.title); SetWindowTextW(app->desc_edit, task.description); Button_SetCheck(app->completed_check, task.completed ? BST_CHECKED : BST_UNCHECKED);
    ComboBox_SetCurSel(app->priority_combo, task.priority - 1); app->selected_task_id = task.id; EnableWindow(app->update_button, TRUE); EnableWindow(app->delete_button, TRUE); SetFocus(app->title_edit);
}

static void add_task(AppState *app) { Task task; wchar_t error[512]; if (!read_form(app, &task)) return; if (!database_insert(&app->database, &task, error, 512)) { log_write(error); MessageBoxW(app->hwnd_main, error, L"Personal ToDo", MB_ICONERROR); return; } reset_form(app); refresh_list(app); }
static void update_task(AppState *app) { Task task; wchar_t error[512]; if (!app->selected_task_id) { MessageBoxW(app->hwnd_main, L"목록에서 수정할 작업을 더블 클릭해 주세요.", L"작업 선택", MB_ICONINFORMATION); return; } if (!read_form(app, &task)) return; if (!database_update(&app->database, &task, error, 512)) { log_write(error); MessageBoxW(app->hwnd_main, error, L"Personal ToDo", MB_ICONERROR); return; } reset_form(app); refresh_list(app); }
static void delete_task(AppState *app) { wchar_t error[512]; if (!app->selected_task_id) { MessageBoxW(app->hwnd_main, L"목록에서 삭제할 작업을 더블 클릭해 주세요.", L"작업 선택", MB_ICONINFORMATION); return; } if (MessageBoxW(app->hwnd_main, L"선택한 작업을 삭제하시겠습니까?", L"삭제 확인", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES) return; if (!database_delete(&app->database, app->selected_task_id, error, 512)) { log_write(error); MessageBoxW(app->hwnd_main, error, L"Personal ToDo", MB_ICONERROR); return; } reset_form(app); refresh_list(app); }

static void save_window_settings(AppState *app) {
    WINDOWPLACEMENT placement; RECT rect;
    ZeroMemory(&placement, sizeof(placement)); placement.length = sizeof(placement);
    if (GetWindowPlacement(app->hwnd_main, &placement)) rect = placement.rcNormalPosition; else GetWindowRect(app->hwnd_main, &rect);
    app->settings.x = rect.left; app->settings.y = rect.top; app->settings.width = rect.right - rect.left; app->settings.height = rect.bottom - rect.top;
    if (!settings_save(app->paths.settings, &app->settings)) log_write(L"Settings save failed");
}

BOOL ui_register_class(HINSTANCE instance) {
    WNDCLASSEXW wc; ZeroMemory(&wc, sizeof(wc)); wc.cbSize = sizeof(wc); wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_proc; wc.hInstance = instance; wc.hCursor = LoadCursorW(NULL, IDC_ARROW); wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); wc.lpszClassName = APP_CLASS; wc.hIconSm = wc.hIcon;
    return RegisterClassExW(&wc) != 0;
}

BOOL ui_create_main_window(AppState *app) {
    app->hwnd_main = CreateWindowExW(WS_EX_LAYERED, APP_CLASS, L"Personal ToDo", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        app->settings.x, app->settings.y, app->settings.width, app->settings.height, NULL, NULL, app->instance, app);
    return app->hwnd_main != NULL;
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    AppState *app = (AppState *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (message == WM_NCCREATE) { CREATESTRUCTW *create = (CREATESTRUCTW *)lparam; app = (AppState *)create->lpCreateParams; app->hwnd_main = hwnd; SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)app); }
    if (!app) return DefWindowProcW(hwnd, message, wparam, lparam);
    switch (message) {
    case WM_CREATE:
        if (!create_controls(app)) return -1;
        Button_SetCheck(app->topmost_check, app->settings.always_on_top ? BST_CHECKED : BST_UNCHECKED); SendMessageW(app->opacity_trackbar, TBM_SETPOS, TRUE, app->settings.opacity);
        apply_opacity(app); apply_topmost(app); reset_form(app); refresh_list(app); return 0;
    case WM_SIZE: layout_controls(app, LOWORD(lparam), HIWORD(lparam)); return 0;
    case WM_GETMINMAXINFO: ((MINMAXINFO *)lparam)->ptMinTrackSize.x = 430; ((MINMAXINFO *)lparam)->ptMinTrackSize.y = 560; return 0;
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case IDC_TIME_CHECK: EnableWindow(app->time_picker, Button_GetCheck(app->time_check) == BST_CHECKED); return 0;
        case IDC_TOPMOST_CHECK: app->settings.always_on_top = Button_GetCheck(app->topmost_check) == BST_CHECKED; apply_topmost(app); return 0;
        case IDC_BTN_ADD: add_task(app); return 0;
        case IDC_BTN_UPDATE: update_task(app); return 0;
        case IDC_BTN_DELETE: delete_task(app); return 0;
        case IDC_BTN_CLEAR: reset_form(app); return 0;
        }
        break;
    case WM_NOTIFY:
        if (((LPNMHDR)lparam)->hwndFrom == ListView_GetHeader(app->task_list) &&
            (((LPNMHDR)lparam)->code == HDN_DIVIDERDBLCLICKW || ((LPNMHDR)lparam)->code == HDN_DIVIDERDBLCLICKA)) {
            /* The header performs automatic sizing after sending this
               notification, so restore the application widths afterward. */
            PostMessageW(hwnd, WM_APP_RESET_COLUMNS, 0, 0);
            return 0;
        }
        if (((LPNMHDR)lparam)->idFrom == IDC_TASK_LIST && ((LPNMHDR)lparam)->code == NM_DBLCLK) { load_selected(app); return 0; }
        break;
    case WM_APP_RESET_COLUMNS: resize_task_columns(app); return 0;
    case WM_HSCROLL:
        if ((HWND)lparam == app->opacity_trackbar) { app->settings.opacity = (int)SendMessageW(app->opacity_trackbar, TBM_GETPOS, 0, 0); apply_opacity(app); return 0; }
        break;
    case WM_CLOSE: save_window_settings(app); DestroyWindow(hwnd); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

#endif /* Legacy V1.5 ui.c */
