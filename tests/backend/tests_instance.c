/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tests_inside.h"

BACKEND_EXPORT backend_instance_t *instance_create(backend_module_t *m,
    const backend_memory_import_t *memories, uint32_t count,
    backend_err_t *err)
{
    backend_instance_t *i = m->stray ? nullptr : calloc(1, sizeof(*i));

    if (!i) {
        snprintf(err->text, sizeof(err->text), "nothing answers its import "
            "'elsewhere::number'");
        return (nullptr);
    }
    i->m = m;
    tests_fill_api(&i->api);
    tests_take_memories(&i->api, memories, count);
    if (i->api.shared_base && i->api.private_base)
        return (i);
    snprintf(err->text, sizeof(err->text), "it was not handed both memories");
    free(i);
    return (nullptr);
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

BACKEND_EXPORT void instance_set_work_limit(backend_instance_t *i,
    uint64_t units)
{
    i->bounded = 1;
    i->work = units;
}

BACKEND_EXPORT void instance_set_timeout(backend_instance_t *i, uint64_t nanos)
{
    i->timeout = nanos;
}

BACKEND_EXPORT void instance_set_memory_limit(backend_instance_t *i,
    uint64_t bytes)
{
    i->memory_limit = bytes;
}
