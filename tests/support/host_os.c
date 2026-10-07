/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/support/host_os.c -- what a host program asks the operating system
** itself, and nothing of ours.
*/

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

#include "host.h"

#ifdef _WIN32

/*
** host_is_library_loaded
**
** Answers whether a library is in this process, asked of the operating
** system.
*/
bool host_is_library_loaded(const char *path)
{
    char full[MAX_PATH];
    DWORD len = GetFullPathNameA(path, sizeof(full), full, nullptr);

    return (len && len < sizeof(full) && GetModuleHandleA(full));
}

#else

/*
** host_is_library_loaded
**
** Answers whether a library is in this process, asked of the operating
** system.
*/
bool host_is_library_loaded(const char *path)
{
    void *lib = dlopen(path, RTLD_NOLOAD | RTLD_NOW);

    if (lib)
        dlclose(lib);
    return (lib != nullptr);
}

#endif
