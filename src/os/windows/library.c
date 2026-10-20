/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <wchar.h>

#include "os_types.h"

static int to_wide(const char *path, wchar_t *wide, int cap)
{
    return (MultiByteToWideChar(CP_UTF8, 0, path, -1, wide, cap));
}

void *os_lib_open(const char *path)
{
    wchar_t wide[MAX_PATH];
    wchar_t full[MAX_PATH];
    DWORD len;
    DWORD beside = LOAD_WITH_ALTERED_SEARCH_PATH;

    if (!to_wide(path, wide, MAX_PATH))
        return (nullptr);
    if (!wcschr(wide, '/') && !wcschr(wide, '\\'))
        return ((void *)LoadLibraryW(wide));
    len = GetFullPathNameW(wide, MAX_PATH, full, nullptr);
    if (!len || len >= MAX_PATH)
        return (nullptr);
    return ((void *)LoadLibraryExW(full, nullptr, beside));
}

void *os_lib_get_symbol(void *lib, const char *name)
{
    return ((void *)GetProcAddress((HMODULE)lib, name));
}

void os_lib_close(void *lib)
{
    FreeLibrary((HMODULE)lib);
}
