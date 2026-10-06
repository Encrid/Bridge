/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** utils/os/windows/library.c -- a shared library on Windows: opened, looked
** into and closed.
*/

#include <string.h>

#include "os_types.h"

/*
** os_lib_open
**
** Opens a library. Windows looks beside the library for what it needs only
** when asked, and only for a full path written its own way, so a path is made
** full first. A bare name is left to the operating system's search.
*/
void *os_lib_open(const char *path)
{
    char full[MAX_PATH];
    DWORD len;
    DWORD beside = LOAD_WITH_ALTERED_SEARCH_PATH;

    if (!strpbrk(path, "/\\"))
        return ((void *)LoadLibraryA(path));
    len = GetFullPathNameA(path, sizeof(full), full, nullptr);
    if (!len || len >= sizeof(full))
        return (nullptr);
    return ((void *)LoadLibraryExA(full, nullptr, beside));
}

/*
** os_lib_get_symbol
**
** Answers where a library has what it exports by `name`, or nullptr.
*/
void *os_lib_get_symbol(void *lib, const char *name)
{
    return ((void *)GetProcAddress((HMODULE)lib, name));
}

/*
** os_lib_close
**
** Closes a library. A null `lib` is nothing to close.
*/
void os_lib_close(void *lib)
{
    if (lib)
        FreeLibrary((HMODULE)lib);
}
