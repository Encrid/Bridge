/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <dlfcn.h>

#include "os_types.h"

void *os_lib_open(const char *path)
{
    int flags = RTLD_NOW | RTLD_LOCAL;

#ifdef C23B_KEEP_LOADED
    flags |= RTLD_NODELETE;
#endif
    return (dlopen(path, flags));
}

void *os_lib_get_symbol(void *lib, const char *name)
{
    return (dlsym(lib, name));
}

void os_lib_close(void *lib)
{
    dlclose(lib);
}
