/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_module.c -- a module opened: a module of the tests
** is its library.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tests_inside.h"

static const char *const TESTS_MEMORIES[2] = {
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
** find_path
**
** Answers the path of a module's library: the path it was handed, or the
** bytes it was handed, which a case of the tests makes the path, of the
** length stated and no longer. nullptr if it was handed neither.
*/
static const char *find_path(const backend_source_t *source, char *named,
    size_t room)
{
    if (source->path && *source->path)
        return (source->path);
    if (!source->bytes || !source->len || source->len >= room)
        return (nullptr);
    memcpy(named, source->bytes, source->len);
    named[source->len] = 0;
    return (named);
}

/*
** module_compile
**
** Opens a module's library.
**
** backend.md, *What a backend is asked for*
*/
BACKEND_EXPORT backend_module_t *module_compile([[maybe_unused]] backend_t *b,
    const backend_source_t *source, backend_err_t *err)
{
    char named[512];
    const char *path = find_path(source, named, sizeof(named));
    void *lib = path ? os_lib_open(path) : nullptr;
    backend_module_t *m;

    if (!path)
        snprintf(err->text, sizeof(err->text), "the tests' backend opens a "
            "module's library by its path, and was handed neither that nor "
            "bytes that are it");
    else if (!lib)
        snprintf(err->text, sizeof(err->text), "cannot open the library "
            "'%.180s'", path);
    if (!lib)
        return (nullptr);
    m = calloc(1, sizeof(*m));
    m->lib = lib;
    m->keep = (void (*)(const test_api_t *))os_lib_get_symbol(lib,
        "test_keep_table");
    m->stray = os_lib_get_symbol(lib, "test_import_stray") != nullptr;
    return (m);
}

/*
** module_list_imports
**
** Writes up to `max` of what a module imports, and answers how many it
** imports. A library has no import section to read, so it is taken to want
** both memories; and one more thing, of another kind, where the module
** says it imports what nobody answers.
*/
BACKEND_EXPORT uint32_t module_list_imports(backend_module_t *m,
    backend_import_t *out, uint32_t max)
{
    uint32_t i = 0;

    while (i < 2 && i < max) {
        out[i] = (backend_import_t){ BRIDGE_NAMESPACE,
            sizeof(BRIDGE_NAMESPACE) - 1, TESTS_MEMORIES[i],
            strlen(TESTS_MEMORIES[i]), BACKEND_IMPORT_MEMORY, {} };
        i++;
    }
    if (m->stray && max > 2)
        out[2] = (backend_import_t){ "elsewhere", 9, "number", 6,
            BACKEND_IMPORT_OTHER, {} };
    return (m->stray ? 3 : 2);
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
