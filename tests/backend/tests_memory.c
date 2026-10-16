/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "tests_inside.h"

BACKEND_EXPORT void memory_get_shape(backend_t *b, uint64_t *reservation,
    uint64_t *guard, uint64_t *granule)
{
    *reservation = b->reservation;
    *guard = b->guard;
    *granule = b->granule;
}

BACKEND_EXPORT backend_memory_t *memory_adopt([[maybe_unused]] backend_t *b,
    void *ptr, uint64_t len, [[maybe_unused]] backend_err_t *err)
{
    backend_memory_t *m = malloc(sizeof(*m));

    m->ptr = ptr;
    m->len = len;
    return (m);
}
