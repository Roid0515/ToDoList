#include "app.h"
#include "ui.h"
#include "log.h"
#include <commctrl.h>

static BOOL validate_window_position(AppSettings *s) {
    RECT rect;
    if (s->x == CW_USEDEFAULT || s->y == CW_USEDEFAULT) return TRUE;
    SetRect(&rect, s->x, s->y, s->x + s->width, s->y + s->height);
    if (!MonitorFromRect(&rect, MONITOR_DEFAULTTONULL)) { s->x = CW_USEDEFAULT; s->y = CW_USEDEFAULT; return FALSE; }
    return TRUE;
}

int app_run(HINSTANCE instance, int show_command) {
    AppState app; INITCOMMONCONTROLSEX controls; MSG message; wchar_t error[512]; int result = 1;
    ZeroMemory(&app, sizeof(app)); app.instance = instance;
    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_LISTVIEW_CLASSES | ICC_DATE_CLASSES | ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&controls);
    if (!paths_initialize(&app.paths)) { MessageBoxW(NULL, L"사용자 데이터 폴더를 만들 수 없습니다.", L"Personal ToDo", MB_ICONERROR); return 1; }
    log_initialize(app.paths.log); log_write(L"Application Start");
    settings_load(app.paths.settings, &app.settings); validate_window_position(&app.settings);
    if (!database_open(&app.database, app.paths.database, error, 512)) { log_write(error); MessageBoxW(NULL, error, L"Personal ToDo", MB_ICONERROR); return 1; }
    if (!ui_register_class(instance) || !ui_create_main_window(&app)) { log_write(L"Main window creation failed"); MessageBoxW(NULL, L"프로그램 창을 만들 수 없습니다.", L"Personal ToDo", MB_ICONERROR); goto cleanup; }
    ShowWindow(app.hwnd_main, show_command); UpdateWindow(app.hwnd_main);
    while (GetMessageW(&message, NULL, 0, 0) > 0) {
        /* A regular top-level Win32 window does not perform dialog-style
           keyboard navigation automatically.  IsDialogMessage handles
           Tab/Shift+Tab traversal for controls marked with WS_TABSTOP. */
        if (!IsDialogMessageW(app.hwnd_main, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    result = (int)message.wParam;
cleanup:
    database_close(&app.database);
    return result;
}
