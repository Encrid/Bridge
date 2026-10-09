/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_instance.c -- an instance made and freed, and the
** three limits Bridge sets on one, which this backend keeps by
** counting what a module says it does.
**
** limits.md, *What bounds a run*
*/

#include <stdlib.h>
#include <string.h>

#include "tests_inside.h"

/*
** instance_create
**
** Makes an instance of a module, with the table it is handed and the two
** memories.
*/
BACKEND_EXPORT backend_instance_t *instance_create(backend_module_t *m,
    const backend_memory_import_t *memories, uint32_t count,
    [[maybe_unused]] backend_err_t *err)
{
    backend_instance_t *i = calloc(1, sizeof(*i));
    uint32_t k = 0;

    i->m = m;
    tests_fill_api(&i->api);
    while (k < count) {
        if (!strcmp(memories[k].field, BRIDGE_SHARED))
            i->api.shared_base = memories[k].memory->ptr;
        if (!strcmp(memories[k].field, BRIDGE_PRIVATE))
            i->api.private_base = memories[k].memory->ptr;
        k++;
    }
    return (i);
}

/*
** instance_destroy
**
** Frees an instance, with the functions found in it.
*/
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

/*
** instance_set_work_limit
**
** States how much work the instance's next calls may say they do.
*/
BACKEND_EXPORT void instance_set_work_limit(backend_instance_t *i,
    uint64_t units)
{
    i->bounded = 1;
    i->work = units;
}

/*
** instance_set_timeout
**
** States how long the next call may say it waits, and that call only.
*/
BACKEND_EXPORT void instance_set_timeout(backend_instance_t *i, uint64_t nanos)
{
    i->timeout = nanos;
}

/*
** instance_set_memory_limit
**
** States the most memory of its own the instance may have.
*/
BACKEND_EXPORT void instance_set_memory_limit(backend_instance_t *i,
    uint64_t bytes)
{
    i->memory_limit = bytes;
}
