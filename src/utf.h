#pragma once
#include <windows.h>

char *utf8_from_wide(const wchar_t *text);
wchar_t *wide_from_utf8(const char *text);

