/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** utils/os/posix/library.c -- a shared library on Linux and macOS: opened,
** looked into and closed.
*/

#include <dlfcn.h>

#include "os_types.h"

/*
** os_lib_open
**
** Opens a library, with its names kept to itself. The loader looks beside a
** library named by a path, and searches for a bare name, without being asked.
*/
void *os_lib_open(const char *path)
{
    return (dlopen(path, RTLD_NOW | RTLD_LOCAL));
}

/*
** os_lib_get_symbol
**
** Answers where a library has what it exports by `name`, or nullptr.
*/
void *os_lib_get_symbol(void *lib, const char *name)
{
    return (dlsym(lib, name));
}

/*
** os_lib_close
**
** Closes a library. A null `lib` is nothing to close.
*/
void os_lib_close(void *lib)
{
    if (lib)
        dlclose(lib);
}
