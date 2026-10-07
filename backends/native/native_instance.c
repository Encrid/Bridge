/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** backends/native/native_instance.c -- an instance made and freed, and the
** three limits this backend keeps none of: it answered no to all three, so
** Bridge-ASM700 never has a number to hand them.
*/

#include <stdlib.h>
#include <string.h>

#include "native_inside.h"

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
** Does nothing: this backend keeps no work limit.
*/
BACKEND_EXPORT void instance_set_work_limit(
    [[maybe_unused]] backend_instance_t *i, [[maybe_unused]] uint64_t units)
{
}

/*
** instance_set_timeout
**
** Does nothing: this backend keeps no timeout.
*/
BACKEND_EXPORT void instance_set_timeout(
    [[maybe_unused]] backend_instance_t *i, [[maybe_unused]] uint64_t nanos)
{
}

/*
** instance_set_memory_limit
**
** Does nothing: this backend keeps no memory limit.
*/
BACKEND_EXPORT void instance_set_memory_limit(
    [[maybe_unused]] backend_instance_t *i, [[maybe_unused]] uint64_t bytes)
{
}
