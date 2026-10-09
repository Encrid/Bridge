/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_memory.c -- the shape asked of a block, and a block
** adopted. A file of its own, so that a case builds this backend with one
** that adopts none in its place.
**
** backend.md, *Memory is adopted, not created*
*/

#include <stdlib.h>

#include "tests_inside.h"

/*
** memory_get_shape
**
** Answers the shape it was told to ask of a memory.
*/
BACKEND_EXPORT void memory_get_shape(backend_t *b, uint64_t *reservation,
    uint64_t *guard, uint64_t *granule)
{
    *reservation = b->reservation;
    *guard = b->guard;
    *granule = b->granule;
}

/*
** memory_adopt
**
** Adopts a block Bridge reserved, which is almost nothing: the pointer is
** already addressable.
*/
BACKEND_EXPORT backend_memory_t *memory_adopt([[maybe_unused]] backend_t *b,
    void *ptr, uint64_t len, [[maybe_unused]] backend_err_t *err)
{
    backend_memory_t *m = malloc(sizeof(*m));

    m->ptr = ptr;
    m->len = len;
    return (m);
}
