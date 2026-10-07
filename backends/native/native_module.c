/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** backends/native/native_module.c -- a module opened: a native module is
** its library.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native_inside.h"

static const char *const NATIVE_MEMORIES[2] = {
    BRIDGE_SHARED,
    BRIDGE_PRIVATE
};

/*
** memory_release
**
** Releases an adopted block.
*/
BACKEND_EXPORT void memory_release(backend_memory_t *m)
{
    free(m);
}

/*
** module_compile
**
** Opens a module's library by its path.
*/
BACKEND_EXPORT backend_module_t *module_compile([[maybe_unused]] backend_t *b,
    const backend_source_t *source, backend_err_t *err)
{
    constexpr size_t room = sizeof(err->text);
    const char *path = source->path;
    void *lib = path && *path ? os_lib_open(path) : nullptr;
    backend_module_t *m;

    if (!path || !*path)
        snprintf(err->text, room, "this backend opens a module's library by "
            "its path, and was handed none");
    else if (!lib)
        snprintf(err->text, room, "cannot open the library '%.180s'", path);
    if (!lib)
        return (nullptr);
    m = calloc(1, sizeof(*m));
    m->lib = lib;
    return (m);
}

/*
** module_list_imports
**
** Writes up to `max` of what a module imports, and answers how many it
** imports. A library has no import section to read, so it is taken to want
** both memories.
*/
BACKEND_EXPORT uint32_t module_list_imports(
    [[maybe_unused]] backend_module_t *m, backend_import_t *out, uint32_t max)
{
    uint32_t i = 0;

    while (i < 2 && i < max) {
        out[i].ns = BRIDGE_NAMESPACE;
        out[i].ns_len = sizeof(BRIDGE_NAMESPACE) - 1;
        out[i].field = NATIVE_MEMORIES[i];
        out[i].field_len = strlen(NATIVE_MEMORIES[i]);
        out[i].kind = BACKEND_IMPORT_MEMORY;
        i++;
    }
    return (2);
}

/*
** module_destroy
**
** Closes a module's library.
*/
BACKEND_EXPORT void module_destroy(backend_module_t *m)
{
    os_lib_close(m->lib);
    free(m);
}
