#include "log.h"
#include <windows.h>
#include <strsafe.h>

static wchar_t g_log_path[MAX_PATH];

void log_initialize(const wchar_t *path) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    StringCchCopyW(g_log_path, MAX_PATH, path);
    if (GetFileAttributesExW(path, GetFileExInfoStandard, &data) && data.nFileSizeHigh == 0 && data.nFileSizeLow > 1024 * 1024) {
        wchar_t backup[MAX_PATH];
        if (SUCCEEDED(StringCchPrintfW(backup, MAX_PATH, L"%s.1", path))) MoveFileExW(path, backup, MOVEFILE_REPLACE_EXISTING);
    }
}

void log_write(const wchar_t *message) {
    HANDLE file; SYSTEMTIME now; wchar_t line[1024]; DWORD bytes;
    if (!g_log_path[0]) return;
    file = CreateFileW(g_log_path, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return;
    GetLocalTime(&now);
    if (SUCCEEDED(StringCchPrintfW(line, 1024, L"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n", now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, message))) {
        int size = WideCharToMultiByte(CP_UTF8, 0, line, -1, NULL, 0, NULL, NULL);
        if (size > 1) {
            char *utf8 = (char *)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)size);
            if (utf8) { WideCharToMultiByte(CP_UTF8, 0, line, -1, utf8, size, NULL, NULL); WriteFile(file, utf8, (DWORD)(size - 1), &bytes, NULL); HeapFree(GetProcessHeap(), 0, utf8); }
        }
    }
    CloseHandle(file);
}

