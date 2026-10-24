/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "bridge_backend.h"

BACKEND_EXPORT enum backend_capability backend_get_capabilities(
    [[maybe_unused]] backend_t *b)
{
    return ((enum backend_capability)0);
}

BACKEND_EXPORT void memory_get_shape([[maybe_unused]] backend_t *b,
    uint64_t *reservation, uint64_t *guard, uint64_t *granule)
{
    *reservation = 0;
    *guard = 0;
    *granule = 0;
}

BACKEND_EXPORT backend_memory_t *memory_adopt([[maybe_unused]] backend_t *b,
    [[maybe_unused]] void *ptr, [[maybe_unused]] uint64_t len,
    backend_err_t *err)
{
    snprintf(err->text, sizeof(err->text), "this backend adopts no block");
    return (nullptr);
}
