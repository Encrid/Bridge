/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/support/native_refusing_memory.c -- the native backend's entries
** about memory, as a backend that cannot adopt a block would answer them:
** for the case in which a backend refuses the shared memory. With
** native_refusing_backend.c it stands in the place of
** backends/native/native_backend.c.
*/

#include <stdio.h>

#include "bridge_backend.h"

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
