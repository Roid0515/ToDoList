#include "ui_internal.h"
#include "log.h"
#include <strsafe.h>
#include <windowsx.h>

void ui_settings_apply_opacity(AppState *app)
{
    BYTE alpha = (BYTE)((app->settings.opacity * 255 + 50) / 100);
    wchar_t text[32];

    SetLayeredWindowAttributes(app->hwnd_main, 0, alpha, LWA_ALPHA);
    if (app->opacity_label) {
        StringCchPrintfW(
            text,
            32,
            L"투명도 %d%%",
            app->settings.opacity);
        SetWindowTextW(app->opacity_label, text);
    }
}

void ui_settings_apply_topmost(AppState *app)
{
    HWND insert_after = app->settings.always_on_top
        ? HWND_TOPMOST
        : HWND_NOTOPMOST;

    SetWindowPos(
        app->hwnd_main,
        insert_after,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void ui_settings_save_window(AppState *app)
{
    WINDOWPLACEMENT placement;
    RECT rect;

    ZeroMemory(&placement, sizeof(placement));
    placement.length = sizeof(placement);

    if (GetWindowPlacement(app->hwnd_main, &placement)) {
        rect = placement.rcNormalPosition;
    } else {
        GetWindowRect(app->hwnd_main, &rect);
    }

    app->settings.x = rect.left;
    app->settings.y = rect.top;
    app->settings.width = rect.right - rect.left;
    app->settings.height = rect.bottom - rect.top;

    if (!settings_save(app->paths.settings, &app->settings)) {
        log_write(L"Settings save failed");
    }
}
