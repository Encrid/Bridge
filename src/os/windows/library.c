/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "os_types.h"

void *os_lib_open(const char *path)
{
    char full[MAX_PATH];
    DWORD len;
    DWORD beside = LOAD_WITH_ALTERED_SEARCH_PATH;

    if (!strchr(path, '/') && !strchr(path, '\\'))
        return ((void *)LoadLibraryA(path));
    len = GetFullPathNameA(path, sizeof(full), full, nullptr);
    if (!len || len >= sizeof(full))
        return (nullptr);
    return ((void *)LoadLibraryExA(full, nullptr, beside));
}

void *os_lib_get_symbol(void *lib, const char *name)
{
    return ((void *)GetProcAddress((HMODULE)lib, name));
}

void os_lib_close(void *lib)
{
    if (lib)
        FreeLibrary((HMODULE)lib);
}
