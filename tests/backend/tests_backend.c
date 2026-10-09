/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_backend.c -- the backend made and freed, what it
** says of itself, and a block adopted.
**
** capabilities.md, *Capabilities, so the swap is honest*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tests_inside.h"

/*
** backend_create
**
** Makes the backend. It has one setting, `capabilities`, which is what it
** then says it can keep: a case states the backend it needs. Any other key
** is one it does not know, and it is not made without that one.
**
** host_api.md, *The data*
*/
BACKEND_EXPORT backend_t *backend_create(const backend_setting_t *settings,
    uint32_t count, backend_err_t *err)
{
    constexpr size_t room = sizeof(err->text);
    const char *key = count && settings[0].key ? settings[0].key : "";
    backend_t *b;

    if (!count)
        snprintf(err->text, room, "it was not told its 'capabilities'");
    else if (count != 1 || strcmp(key, "capabilities"))
        snprintf(err->text, room, "it knows no setting '%.100s'", key);
    if (count != 1 || strcmp(key, "capabilities"))
        return (nullptr);
    b = calloc(1, sizeof(*b));
    b->capabilities = (enum backend_capability)settings[0].v.i;
    return (b);
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
** Answers what it was told it can keep.
*/
BACKEND_EXPORT enum backend_capability backend_get_capabilities(backend_t *b)
{
    return (b->capabilities);
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
