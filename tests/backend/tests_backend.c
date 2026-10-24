/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tests_inside.h"

static int take_setting(backend_t *b, const backend_setting_t *s)
{
    static const char *const keys[4] = { "capabilities", "reservation",
        "guard", "granule" };
    uint64_t *into[4] = { nullptr, &b->reservation, &b->guard, &b->granule };
    uint32_t k = 0;

    while (k < 4 && (!s->key || strcmp(s->key, keys[k])))
        k++;
    if (k == 4)
        return (0);
    if (!k)
        b->capabilities = (enum backend_capability)s->v.i;
    else
        *into[k] = (uint64_t)s->v.i;
    b->told |= 1u << k;
    return (1);
}

BACKEND_EXPORT backend_t *backend_create(const backend_setting_t *settings,
    uint32_t count, backend_err_t *err)
{
    constexpr size_t room = sizeof(err->text);
    backend_t *b = calloc(1, sizeof(*b));
    uint32_t k = 0;

    while (k < count && take_setting(b, settings + k))
        k++;
    if (k < count)
        snprintf(err->text, room, "it knows no setting '%.100s'",
            settings[k].key ? settings[k].key : "");
    else if (b->told != 15u)
        snprintf(err->text, room, "it was not told its 'capabilities', and "
            "the shape of a block it asks for");
    if (k == count && b->told == 15u)
        return (b);
    free(b);
    return (nullptr);
}

BACKEND_EXPORT void backend_destroy(backend_t *b)
{
    free(b);
}

BACKEND_EXPORT enum backend_capability backend_get_capabilities(backend_t *b)
{
    return (b->capabilities);
}
