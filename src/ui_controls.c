#include "ui_internal.h"
#include "resource.h"
#include <windowsx.h>

const ListColumnDef g_task_columns[UI_TASK_COLUMN_COUNT] = {
    {L"우선순위", 68},
    {L"날짜", 88},
    {L"시간", 55},
    {L"제목", 220},
    {L"상태", 58}
};

static HWND create_control(
    HWND parent,
    const wchar_t *class_name,
    const wchar_t *text,
    DWORD style,
    int id)
{
    HWND item = CreateWindowExW(
        0,
        class_name,
        text,
        WS_CHILD | WS_VISIBLE | style,
        0,
        0,
        0,
        0,
        parent,
        (HMENU)(INT_PTR)id,
        GetModuleHandleW(NULL),
        NULL);

    if (item) {
        SendMessageW(item, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    }
    return item;
}

static HWND create_label(HWND parent, const wchar_t *text)
{
    return create_control(parent, L"STATIC", text, SS_LEFT, 0);
}

static void create_task_columns(HWND list_view)
{
    LVCOLUMNW column;
    int index;

    ZeroMemory(&column, sizeof(column));
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    for (index = 0; index < UI_TASK_COLUMN_COUNT; ++index) {
        column.iSubItem = index;
        column.pszText = (LPWSTR)g_task_columns[index].name;
        column.cx = g_task_columns[index].width;
        ListView_InsertColumn(list_view, index, &column);
    }
}

static void initialize_priority_combo(HWND combo)
{
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"긴급");
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"높음");
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"보통");
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"낮음");
}

BOOL ui_controls_create(AppState *app)
{
    app->date_label = create_label(app->hwnd_main, L"날짜");
    app->date_picker = create_control(
        app->hwnd_main,
        DATETIMEPICK_CLASSW,
        L"",
        DTS_SHORTDATEFORMAT | WS_TABSTOP,
        IDC_DATE_PICKER);
    app->time_check = create_control(
        app->hwnd_main,
        L"BUTTON",
        L"시간 지정",
        BS_AUTOCHECKBOX | WS_TABSTOP,
        IDC_TIME_CHECK);
    app->time_picker = create_control(
        app->hwnd_main,
        DATETIMEPICK_CLASSW,
        L"",
        DTS_TIMEFORMAT | WS_TABSTOP,
        IDC_TIME_PICKER);
    SendMessageW(app->time_picker, DTM_SETFORMATW, 0, (LPARAM)L"HH:mm");

    app->title_label = create_label(app->hwnd_main, L"제목 *");
    app->title_edit = create_control(
        app->hwnd_main,
        L"EDIT",
        L"",
        WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
        IDC_TITLE_EDIT);
    SendMessageW(app->title_edit, EM_SETLIMITTEXT, TASK_TITLE_CAP - 1, 0);

    app->desc_label = create_label(app->hwnd_main, L"내용");
    app->desc_edit = create_control(
        app->hwnd_main,
        L"EDIT",
        L"",
        WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_TABSTOP,
        IDC_DESC_EDIT);
    SendMessageW(app->desc_edit, EM_SETLIMITTEXT, TASK_DESC_CAP - 1, 0);

    app->priority_label = create_label(app->hwnd_main, L"우선순위");
    app->priority_combo = create_control(
        app->hwnd_main,
        L"COMBOBOX",
        L"",
        CBS_DROPDOWNLIST | WS_TABSTOP,
        IDC_PRIORITY_COMBO);
    initialize_priority_combo(app->priority_combo);

    app->completed_check = create_control(
        app->hwnd_main,
        L"BUTTON",
        L"완료",
        BS_AUTOCHECKBOX | WS_TABSTOP,
        IDC_COMPLETED_CHECK);
    app->add_button = create_control(
        app->hwnd_main,
        L"BUTTON",
        L"등록",
        BS_PUSHBUTTON | WS_TABSTOP,
        IDC_BTN_ADD);
    app->update_button = create_control(
        app->hwnd_main,
        L"BUTTON",
        L"수정",
        BS_PUSHBUTTON | WS_TABSTOP,
        IDC_BTN_UPDATE);
    app->delete_button = create_control(
        app->hwnd_main,
        L"BUTTON",
        L"삭제",
        BS_PUSHBUTTON | WS_TABSTOP,
        IDC_BTN_DELETE);
    app->clear_button = create_control(
        app->hwnd_main,
        L"BUTTON",
        L"새로 입력",
        BS_PUSHBUTTON | WS_TABSTOP,
        IDC_BTN_CLEAR);

    app->task_list = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        WC_LISTVIEWW,
        L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL |
            LVS_SHOWSELALWAYS,
        0,
        0,
        0,
        0,
        app->hwnd_main,
        (HMENU)(INT_PTR)IDC_TASK_LIST,
        app->instance,
        NULL);
    SendMessageW(
        app->task_list,
        WM_SETFONT,
        (WPARAM)GetStockObject(DEFAULT_GUI_FONT),
        TRUE);
    ListView_SetExtendedListViewStyle(
        app->task_list,
        LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);
    create_task_columns(app->task_list);

    app->topmost_check = create_control(
        app->hwnd_main,
        L"BUTTON",
        L"항상 위",
        BS_AUTOCHECKBOX | WS_TABSTOP,
        IDC_TOPMOST_CHECK);
    app->opacity_label = create_label(app->hwnd_main, L"투명도");
    app->opacity_trackbar = create_control(
        app->hwnd_main,
        TRACKBAR_CLASSW,
        L"",
        TBS_HORZ | TBS_AUTOTICKS | WS_TABSTOP,
        IDC_OPACITY_TRACKBAR);
    SendMessageW(
        app->opacity_trackbar,
        TBM_SETRANGE,
        TRUE,
        MAKELPARAM(30, 100));
    SendMessageW(app->opacity_trackbar, TBM_SETTICFREQ, 10, 0);

    return app->date_label && app->date_picker && app->title_label &&
        app->title_edit && app->task_list && app->opacity_trackbar;
}

void ui_controls_reset_form(AppState *app)
{
    SYSTEMTIME now;

    GetLocalTime(&now);
    DateTime_SetSystemtime(app->date_picker, GDT_VALID, &now);
    DateTime_SetSystemtime(app->time_picker, GDT_VALID, &now);
    Button_SetCheck(app->time_check, BST_UNCHECKED);
    EnableWindow(app->time_picker, FALSE);
    SetWindowTextW(app->title_edit, L"");
    SetWindowTextW(app->desc_edit, L"");
    Button_SetCheck(app->completed_check, BST_UNCHECKED);
    ComboBox_SetCurSel(app->priority_combo, 2);
    app->selected_task_id = 0;
    EnableWindow(app->update_button, FALSE);
    EnableWindow(app->delete_button, FALSE);
    SetFocus(app->title_edit);
}
