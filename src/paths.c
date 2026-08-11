#include "paths.h"
#include <shlobj.h>
#include <strsafe.h>

static BOOL make_dir(const wchar_t *path) {
    int result = SHCreateDirectoryExW(NULL, path, NULL);
    return result == ERROR_SUCCESS || result == ERROR_ALREADY_EXISTS;
}

BOOL paths_initialize(AppPaths *paths) {
    PWSTR local = NULL;
    BOOL ok = FALSE;
    if (!paths || FAILED(SHGetKnownFolderPath(&FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &local))) return FALSE;
    if (SUCCEEDED(StringCchPrintfW(paths->root, MAX_PATH, L"%s\\PersonalTodo", local)) &&
        SUCCEEDED(StringCchPrintfW(paths->data, MAX_PATH, L"%s\\data", paths->root)) &&
        SUCCEEDED(StringCchPrintfW(paths->config, MAX_PATH, L"%s\\config", paths->root)) &&
        SUCCEEDED(StringCchPrintfW(paths->logs, MAX_PATH, L"%s\\logs", paths->root)) &&
        make_dir(paths->root) && make_dir(paths->data) && make_dir(paths->config) && make_dir(paths->logs) &&
        SUCCEEDED(StringCchPrintfW(paths->database, MAX_PATH, L"%s\\todo.db", paths->data)) &&
        SUCCEEDED(StringCchPrintfW(paths->settings, MAX_PATH, L"%s\\settings.ini", paths->config)) &&
        SUCCEEDED(StringCchPrintfW(paths->log, MAX_PATH, L"%s\\app.log", paths->logs))) ok = TRUE;
    CoTaskMemFree(local);
    return ok;
}

