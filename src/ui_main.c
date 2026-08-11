#include "ui.h"
#include "ui_internal.h"
#include "resource.h"
#include "log.h"
#include <windowsx.h>

static LRESULT CALLBACK window_proc(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam);

static BOOL handle_command(AppState *app, int command_id)
{
    switch (command_id) {
    case IDC_TIME_CHECK:
        EnableWindow(
            app->time_picker,
            Button_GetCheck(app->time_check) == BST_CHECKED);
        return TRUE;

    case IDC_TOPMOST_CHECK:
        app->settings.always_on_top =
            Button_GetCheck(app->topmost_check) == BST_CHECKED;
        ui_settings_apply_topmost(app);
        return TRUE;

    case IDC_BTN_ADD:
        ui_tasks_add(app);
        return TRUE;

    case IDC_BTN_UPDATE:
        ui_tasks_update(app);
        return TRUE;

    case IDC_BTN_DELETE:
        ui_tasks_delete(app);
        return TRUE;

    case IDC_BTN_CLEAR:
        ui_controls_reset_form(app);
        return TRUE;

    default:
        return FALSE;
    }
}

static BOOL handle_notify(AppState *app, LPNMHDR notification)
{
    HWND header = ListView_GetHeader(app->task_list);

    if (notification->hwndFrom == header &&
        (notification->code == HDN_DIVIDERDBLCLICKW ||
         notification->code == HDN_DIVIDERDBLCLICKA)) {
        PostMessageW(app->hwnd_main, UI_WM_RESET_COLUMNS, 0, 0);
        return TRUE;
    }

    if (notification->idFrom == IDC_TASK_LIST &&
        notification->code == NM_DBLCLK) {
        ui_tasks_load_selected(app);
        return TRUE;
    }

    return FALSE;
}

static BOOL initialize_window(AppState *app)
{
    if (!ui_controls_create(app)) {
        return FALSE;
    }

    Button_SetCheck(
        app->topmost_check,
        app->settings.always_on_top ? BST_CHECKED : BST_UNCHECKED);
    SendMessageW(
        app->opacity_trackbar,
        TBM_SETPOS,
        TRUE,
        app->settings.opacity);
    ui_settings_apply_opacity(app);
    ui_settings_apply_topmost(app);
    ui_controls_reset_form(app);
    ui_tasks_refresh(app);
    return TRUE;
}

BOOL ui_register_class(HINSTANCE instance)
{
    WNDCLASSEXW window_class;

    ZeroMemory(&window_class, sizeof(window_class));
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursorW(NULL, IDC_ARROW);
    window_class.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    window_class.lpszClassName = UI_APP_CLASS;
    window_class.hIconSm = window_class.hIcon;

    return RegisterClassExW(&window_class) != 0;
}

BOOL ui_create_main_window(AppState *app)
{
    app->hwnd_main = CreateWindowExW(
        WS_EX_LAYERED,
        UI_APP_CLASS,
        L"Personal ToDo",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        app->settings.x,
        app->settings.y,
        app->settings.width,
        app->settings.height,
        NULL,
        NULL,
        app->instance,
        app);

    return app->hwnd_main != NULL;
}

static LRESULT CALLBACK window_proc(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam)
{
    AppState *app = (AppState *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if (message == WM_NCCREATE) {
        CREATESTRUCTW *create = (CREATESTRUCTW *)lparam;

        app = (AppState *)create->lpCreateParams;
        app->hwnd_main = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)app);
    }

    if (!app) {
        return DefWindowProcW(hwnd, message, wparam, lparam);
    }

    switch (message) {
    case WM_CREATE:
        return initialize_window(app) ? 0 : -1;

    case WM_SIZE:
        ui_layout_controls(app, LOWORD(lparam), HIWORD(lparam));
        return 0;

    case WM_GETMINMAXINFO:
        ((MINMAXINFO *)lparam)->ptMinTrackSize.x = 430;
        ((MINMAXINFO *)lparam)->ptMinTrackSize.y = 560;
        return 0;

    case WM_COMMAND:
        if (handle_command(app, LOWORD(wparam))) {
            return 0;
        }
        break;

    case WM_NOTIFY:
        if (handle_notify(app, (LPNMHDR)lparam)) {
            return 0;
        }
        break;

    case WM_HSCROLL:
        if ((HWND)lparam == app->opacity_trackbar) {
            app->settings.opacity = (int)SendMessageW(
                app->opacity_trackbar,
                TBM_GETPOS,
                0,
                0);
            ui_settings_apply_opacity(app);
            return 0;
        }
        break;

    case UI_WM_RESET_COLUMNS:
        ui_layout_reset_task_columns(app);
        return 0;

    case WM_CLOSE:
        ui_settings_save_window(app);
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wparam, lparam);
}
