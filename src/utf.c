#include "utf.h"
#include <stdlib.h>

char *utf8_from_wide(const wchar_t *text) {
    int size; char *out;
    if (!text) return NULL;
    size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text, -1, NULL, 0, NULL, NULL);
    if (!size) return NULL;
    out = (char *)malloc((size_t)size);
    if (!out || !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text, -1, out, size, NULL, NULL)) { free(out); return NULL; }
    return out;
}

wchar_t *wide_from_utf8(const char *text) {
    int size; wchar_t *out;
    if (!text) return NULL;
    size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, NULL, 0);
    if (!size) return NULL;
    out = (wchar_t *)malloc((size_t)size * sizeof(wchar_t));
    if (!out || !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, out, size)) { free(out); return NULL; }
    return out;
}

