/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/reload_record.c -- the record of the instance a reload brings in,
** which takes over what was set aside for the module.
*/

#include <stdlib.h>
#include <string.h>

#include "reload.h"

/*
** alloc_record
**
** Makes the new instance's record, with room for the ranges of the old
** one's share and one more, and for the ids held ready for the old one: an
** array of its own that holds them all, whatever `spawns` it now states.
** nullptr with a record said.
*/
static module_t *alloc_record(reloading_t *r)
{
    const module_t *old = r->old;
    module_t *n = module_record(r->b, r->a, r->on);
    uint64_t *ids = nullptr;

    if (!n)
        return (nullptr);
    n->owned = calloc(old->own_count + 1, sizeof(*n->owned));
    if (n->id_room < old->id_count)
        ids = realloc(n->ids, old->id_count * sizeof(*ids));
    if (ids) {
        n->ids = ids;
        n->id_room = old->id_count;
    }
    if (n->owned && n->id_room >= old->id_count)
        return (n);
    (void)bridge_lacks(r->b, nullptr, "a reloaded module's record");
    module_discard(r->b, n);
    return (nullptr);
}

/*
** take_ids
**
** Copies the ids held ready for the old instance, which are the new one's
** from the swap on, and the settings its config seeded.
*/
static void take_ids(module_t *n, const module_t *old)
{
    if (old->id_count)
        memcpy(n->ids, old->ids, old->id_count * sizeof(*n->ids));
    n->id_count = old->id_count;
    n->id_next = old->id_next;
    n->config = old->config;
    n->text_at = old->text_at;
    n->text_room = old->text_room;
}

/*
** take_share
**
** Copies the old instance's share of the persistent arena, with what the
** reload added to it. A module that had none starts where the range it was
** given is.
*/
static void take_share(const reloading_t *r, module_t *n, const module_t *old)
{
    uint32_t at = r->b->arena + r->extra_at;

    n->arena_share = old->arena_share + r->extra;
    n->arena_at = old->arena_at;
    n->arena_left = old->arena_left + (r->in_place ? r->extra : 0);
    if (old->own_count)
        memcpy(n->owned, old->owned, old->own_count * sizeof(*n->owned));
    n->own_count = old->own_count;
    n->own_next = old->own_next;
    if (r->in_place)
        n->owned[n->own_count - 1].len += r->extra;
    if (r->in_place || !r->extra)
        return;
    n->owned[n->own_count++] = (range_t){ at, r->extra };
    if (old->own_count)
        return;
    n->arena_at = at;
    n->arena_left = r->extra;
    n->own_next = 1;
}

/*
** reload_make_record
**
** Makes the new instance's record, which takes over what was set aside for
** the module.
*/
int reload_make_record(reloading_t *r)
{
    r->n = alloc_record(r);
    if (!r->n) {
        reload_give_back(r);
        return (1);
    }
    take_ids(r->n, r->old);
    take_share(r, r->n, r->old);
    return (0);
}
