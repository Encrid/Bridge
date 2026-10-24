/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tests_inside.h"

static const char *const TESTS_MEMORIES[2] = {
    BRIDGE_SHARED,
    BRIDGE_PRIVATE
};

static const backend_import_t TESTS_STRAYS[3] = {
    { "elsewhere", 9, "number", 6, BACKEND_IMPORT_OTHER, {} },
    { "bridgf", 6, "shared", 6, BACKEND_IMPORT_MEMORY, {} },
    { "bridge", 6, "spare", 5, BACKEND_IMPORT_MEMORY, {} }
};

constexpr uint32_t TESTS_CROWD = 64;

BACKEND_EXPORT void memory_release(backend_memory_t *m)
{
    free(m);
}

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
    m->crowd = os_lib_get_symbol(lib, "test_import_crowd") != nullptr;
    m->insists = os_lib_get_symbol(lib, "test_enter_after_refusal") != nullptr;
    m->undefined = os_lib_get_symbol(lib, "test_refuse_define") != nullptr;
    return (m);
}

static uint32_t count_imports(const backend_module_t *m)
{
    if (m->crowd)
        return (TESTS_CROWD + 2);
    return (m->stray ? 6 : 2);
}

static backend_import_t get_import(const backend_module_t *m, uint32_t i)
{
    uint32_t first = m->stray ? 3 : 0;
    const char *field;

    if (m->crowd)
        first = TESTS_CROWD;
    if (i < first)
        return (TESTS_STRAYS[m->crowd ? 0 : i]);
    if (i >= first + 2)
        return (TESTS_STRAYS[0]);
    field = TESTS_MEMORIES[i - first];
    return ((backend_import_t){ BRIDGE_NAMESPACE,
        sizeof(BRIDGE_NAMESPACE) - 1, field, strlen(field),
        BACKEND_IMPORT_MEMORY, {} });
}

BACKEND_EXPORT uint32_t module_list_imports(backend_module_t *m,
    backend_import_t *out, uint32_t max)
{
    uint32_t count = count_imports(m);
    uint32_t i = 0;

    while (i < count && i < max) {
        out[i] = get_import(m, i);
        i++;
    }
    return (count);
}

BACKEND_EXPORT void module_destroy(backend_module_t *m)
{
    os_lib_close(m->lib);
    free(m);
}
