#pragma once
#include <wchar.h>
void log_initialize(const wchar_t *path);
void log_write(const wchar_t *message);
