/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_APPLY_H
    #define BRIDGE_SRC_STORAGE_APPLY_H

    #include "storage_place.h"
    #include "storage_room.h"

[[gnu::always_inline]] static inline void reserve_all(storage_t *r)
{
    uint32_t i = 0;

    while (i < r->changed_count) {
        reserve_one(r, r->changed[i]);
        i++;
    }
    i = 0;
    while (i < r->changed_count) {
        room_one(r, r->changed[i]);
        i++;
    }
}

static inline void tell_all(storage_t *r)
{
    seen_t seen = { .before = r->before };
    uint32_t index;
    uint32_t i = 0;

    while (r->observer_count && i < r->changed_count) {
        index = r->changed[i];
        seen.id = (uint64_t)r->places[index].generation << 32 | index;
        seen.after = get_work(r, index);
        if (r->folded[index] & DESTROYED)
            seen.after = nullptr;
        seen.attached = get_attached(r, index);
        if (!(r->folded[index] & DROPPED)) {
            get_carried(r, index, r->before);
            tell_observers(r, &seen);
        }
        i++;
    }
}

static inline void place_all(storage_t *r)
{
    uint32_t i = 0;

    while (i < r->changed_count) {
        if (!(r->folded[r->changed[i]] & DROPPED))
            place_one(r, r->changed[i]);
        i++;
    }
}

#endif
