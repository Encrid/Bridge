/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "reload.h"

static void grow_in_place(reloading_t *r)
{
    const module_t *old = r->old;
    region_t *space = &r->b->arena_space;
    uint32_t at;

    if (!old->own_count || old->own_next != old->own_count)
        return;
    r->last = old->owned[old->own_count - 1];
    at = r->last.at - r->b->arena;
    r->in_place = region_grow(space, at, r->last.len, r->last.len + r->extra);
}

static int take_range(reloading_t *r, int64_t wants, uint64_t extra)
{
    bridge_t *b = r->b;
    uint32_t left = region_get_most(&b->arena_space) + r->old->arena_share;

    if (!region_make_room(&b->arena_space, b->arena_ranges + 1))
        return (bridge_lacks(b, nullptr, "one more range of the persistent "
            "arena"));
    if (extra > UINT32_MAX
        || !region_take(&b->arena_space, (uint32_t)extra, &r->extra_at))
        return (reload_refuse(r, "its persistent of %lld bytes does not fit "
            "in the %u left of [budget] persistent", (long long)wants, left));
    b->arena_ranges++;
    return (0);
}

int reload_grow_share(reloading_t *r)
{
    const module_t *old = r->old;
    int64_t had = old->persistent > 0 ? old->persistent : 0;
    int64_t wants = r->a->cfg.persistent > 0 ? r->a->cfg.persistent : 0;
    uint64_t extra = module_round_share(wants) - old->arena_share;

    if (wants < had)
        return (reload_refuse(r, "its persistent is %lld and a reload states "
            "less, which a loaded module's share cannot become",
            (long long)had));
    if (!extra)
        return (0);
    if (extra <= UINT32_MAX) {
        r->extra = (uint32_t)extra;
        grow_in_place(r);
    }
    return (r->in_place ? 0 : take_range(r, wants, extra));
}

void reload_give_back(reloading_t *r)
{
    bridge_t *b = r->b;
    uint32_t end = r->last.at - b->arena + r->last.len;

    if (r->in_place) {
        region_give(&b->arena_space, end, r->extra);
        return;
    }
    if (!r->extra)
        return;
    region_give(&b->arena_space, r->extra_at, r->extra);
    b->arena_ranges--;
}
