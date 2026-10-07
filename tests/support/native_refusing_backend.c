/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/support/native_refusing_backend.c -- the native backend's first
** entries, as a backend that cannot adopt a block would answer them: for
** the case in which a backend refuses the shared memory. It stands in the
** place of backends/native/native_backend.c.
*/

#include <stdio.h>
#include <stdlib.h>

#include "bridge_backend.h"

/*
** backend_create
**
** Makes the backend, whatever its settings: a byte, since it keeps nothing.
*/
BACKEND_EXPORT backend_t *backend_create(
    [[maybe_unused]] const backend_setting_t *settings,
    [[maybe_unused]] uint32_t count, [[maybe_unused]] backend_err_t *err)
{
    return (calloc(1, 1));
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
** Answers that this backend can keep nothing.
*/
BACKEND_EXPORT enum backend_capability backend_get_capabilities(
    [[maybe_unused]] backend_t *b)
{
    return ((enum backend_capability)0);
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
** Refuses the block, and says why.
*/
BACKEND_EXPORT backend_memory_t *memory_adopt([[maybe_unused]] backend_t *b,
    [[maybe_unused]] void *ptr, [[maybe_unused]] uint64_t len,
    backend_err_t *err)
{
    snprintf(err->text, sizeof(err->text), "this backend adopts no block");
    return (nullptr);
}
