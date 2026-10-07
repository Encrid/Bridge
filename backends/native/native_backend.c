/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** backends/native/native_backend.c -- the backend made and freed, what it
** says of itself, and a block adopted.
**
** capabilities.md, *Capabilities, so the swap is honest*
*/

#include <stdio.h>
#include <stdlib.h>

#include "native_inside.h"

/*
** backend_create
**
** Makes the backend. It has no setting of its own, so any key is one it
** does not know.
**
** host_api.md, *The data*
*/
BACKEND_EXPORT backend_t *backend_create(const backend_setting_t *settings,
    uint32_t count, backend_err_t *err)
{
    const char *key = count && settings[0].key ? settings[0].key : "";

    if (!count)
        return (calloc(1, sizeof(backend_t)));
    snprintf(err->text, sizeof(err->text), "it knows no setting '%.100s'", key);
    return (nullptr);
}

/*
** backend_destroy
**
** Frees the backend.
*/
BACKEND_EXPORT void backend_destroy(backend_t *b)
{
    free(b);
}

/*
** backend_get_capabilities
**
** Answers what this backend can keep. It runs only what is trusted, and
** keeps no work limit, no timeout and no memory limit: nothing contains
** code in this process. And not side by side: a process cannot hold two
** copies of one library.
*/
BACKEND_EXPORT enum backend_capability backend_get_capabilities(
    [[maybe_unused]] backend_t *b)
{
    return (BACKEND_RELOAD | BACKEND_UNLOAD);
}

/*
** memory_get_shape
**
** Answers the shape this backend asks of a memory, which is none.
*/
BACKEND_EXPORT void memory_get_shape([[maybe_unused]] backend_t *b,
    uint64_t *reservation, uint64_t *guard, uint64_t *granule)
{
    *reservation = 0;
    *guard = 0;
    *granule = 0;
}

/*
** memory_adopt
**
** Adopts a block Bridge reserved, which is almost nothing: the
** pointer is already addressable.
*/
BACKEND_EXPORT backend_memory_t *memory_adopt([[maybe_unused]] backend_t *b,
    void *ptr, uint64_t len, [[maybe_unused]] backend_err_t *err)
{
    backend_memory_t *m = malloc(sizeof(*m));

    m->ptr = ptr;
    m->len = len;
    return (m);
}
