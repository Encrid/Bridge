/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <dlfcn.h>

#include "os_types.h"

void *os_lib_open(const char *path)
{
    return (dlopen(path, RTLD_NOW | RTLD_LOCAL));
}

void *os_lib_get_symbol(void *lib, const char *name)
{
    return (dlsym(lib, name));
}

void os_lib_close(void *lib)
{
#ifdef C23B_FLUSH_BEFORE_CLOSE
    void (*flush)(void);

    *(void **)&flush = os_lib_get_symbol(lib, "c23b_flush");
    flush();
#endif
    dlclose(lib);
}
