/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "native_inside.h"

BACKEND_EXPORT backend_instance_t *instance_create(backend_module_t *m,
    const backend_memory_import_t *memories, uint32_t count,
    [[maybe_unused]] backend_err_t *err)
{
    backend_instance_t *i = calloc(1, sizeof(*i));
    uint32_t k = 0;

    i->m = m;
    native_fill_api(&i->api);
    while (k < count) {
        if (!strcmp(memories[k].field, BRIDGE_SHARED))
            i->api.shared_base = memories[k].memory->ptr;
        if (!strcmp(memories[k].field, BRIDGE_PRIVATE))
            i->api.private_base = memories[k].memory->ptr;
        k++;
    }
    return (i);
}

BACKEND_EXPORT void instance_destroy(backend_instance_t *i)
{
    backend_func_t *f = i->funcs;

    while (f) {
        i->funcs = f->next;
        free(f);
        f = i->funcs;
    }
    free(i);
}

BACKEND_EXPORT void instance_set_work_limit(
    [[maybe_unused]] backend_instance_t *i, [[maybe_unused]] uint64_t units)
{
}

BACKEND_EXPORT void instance_set_timeout(
    [[maybe_unused]] backend_instance_t *i, [[maybe_unused]] uint64_t nanos)
{
}

BACKEND_EXPORT void instance_set_memory_limit(
    [[maybe_unused]] backend_instance_t *i, [[maybe_unused]] uint64_t bytes)
{
}
