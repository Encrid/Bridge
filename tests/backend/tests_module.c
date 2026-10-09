/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_module.c -- a module found: a module of the tests is
** a name, and what the build compiled in under it.
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
** module_compile
**
** Finds a module by the name its path states.
**
** backend.md, *What a backend is asked for*
*/
BACKEND_EXPORT backend_module_t *module_compile([[maybe_unused]] backend_t *b,
    const backend_source_t *source, backend_err_t *err)
{
    const char *name = source->path ? source->path : "";
    backend_module_t *m;
    uint32_t k = 0;

    while (k < TESTS_MODULE_COUNT && strcmp(TESTS_MODULES[k], name))
        k++;
    if (k == TESTS_MODULE_COUNT) {
        snprintf(err->text, sizeof(err->text), "no module '%.150s' is "
            "compiled into the tests' backend", name);
        return (nullptr);
    }
    m = calloc(1, sizeof(*m));
    m->name = TESTS_MODULES[k];
    return (m);
}

/*
** module_list_imports
**
** Writes up to `max` of what a module imports, and answers how many it
** imports. Compiled code has no import section to read, so it is taken to
** want both memories.
*/
BACKEND_EXPORT uint32_t module_list_imports(
    [[maybe_unused]] backend_module_t *m, backend_import_t *out, uint32_t max)
{
    uint32_t i = 0;

    while (i < 2 && i < max) {
        out[i].ns = BRIDGE_NAMESPACE;
        out[i].ns_len = sizeof(BRIDGE_NAMESPACE) - 1;
        out[i].field = TESTS_MEMORIES[i];
        out[i].field_len = strlen(TESTS_MEMORIES[i]);
        out[i].kind = BACKEND_IMPORT_MEMORY;
        i++;
    }
    return (2);
}

/*
** module_destroy
**
** Frees a module.
*/
BACKEND_EXPORT void module_destroy(backend_module_t *m)
{
    free(m);
}
