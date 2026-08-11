#include "settings.h"
#include <strsafe.h>

void settings_defaults(AppSettings *s) {
    s->x = CW_USEDEFAULT; s->y = CW_USEDEFAULT; s->width = 520; s->height = 720;
    s->opacity = 90; s->always_on_top = 1;
}

void settings_load(const wchar_t *path, AppSettings *s) {
    settings_defaults(s);
    s->x = GetPrivateProfileIntW(L"Window", L"X", s->x, path);
    s->y = GetPrivateProfileIntW(L"Window", L"Y", s->y, path);
    s->width = GetPrivateProfileIntW(L"Window", L"Width", s->width, path);
    s->height = GetPrivateProfileIntW(L"Window", L"Height", s->height, path);
    s->opacity = GetPrivateProfileIntW(L"Window", L"Opacity", s->opacity, path);
    s->always_on_top = GetPrivateProfileIntW(L"Window", L"AlwaysOnTop", s->always_on_top, path);
    if (s->width < 430) s->width = 520;
    if (s->height < 560) s->height = 720;
    if (s->opacity < 30 || s->opacity > 100) s->opacity = 90;
}

static BOOL write_int(const wchar_t *path, const wchar_t *key, int value) {
    wchar_t buffer[32];
    if (FAILED(StringCchPrintfW(buffer, 32, L"%d", value))) return FALSE;
    return WritePrivateProfileStringW(L"Window", key, buffer, path);
}

BOOL settings_save(const wchar_t *path, const AppSettings *s) {
    return write_int(path, L"X", s->x) && write_int(path, L"Y", s->y) &&
        write_int(path, L"Width", s->width) && write_int(path, L"Height", s->height) &&
        write_int(path, L"Opacity", s->opacity) && write_int(path, L"AlwaysOnTop", s->always_on_top);
}

