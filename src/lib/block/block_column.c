/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "block.h"

constexpr uint32_t GRAIN = 16;

static uint32_t round_up(uint32_t size)
{
    return ((size + GRAIN - 1) & ~(GRAIN - 1));
}

void block_column_free(bridge_world_t *w, void *ptr, uint32_t size)
{
    uint32_t at = (uint32_t)((uint8_t *)ptr - w->shared) - w->columns;

    region_give(&w->column_space, at, round_up(size));
}

static bool resize_in_place(bridge_world_t *w, void *ptr, uint32_t had,
    uint32_t want)
{
    uint32_t at = (uint32_t)((uint8_t *)ptr - w->shared) - w->columns;

    if (want < had)
        region_give(&w->column_space, at + want, had - want);
    if (want <= had)
        return (true);
    return (region_grow(&w->column_space, at, had, want));
}

void *block_column_regrow(bridge_world_t *w, void *ptr, uint32_t old_size,
    uint32_t new_size)
{
    uint8_t *base = w->shared + w->columns;
    uint32_t had = round_up(old_size);
    uint32_t want = round_up(new_size);
    uint32_t to;

    if (ptr && resize_in_place(w, ptr, had, want))
        return (ptr);
    if (!region_take(&w->column_space, want, &to))
        return (nullptr);
    if (ptr) {
        memcpy(base + to, ptr, had);
        region_give(&w->column_space, (uint32_t)((uint8_t *)ptr - base), had);
    }
    return (base + to);
}
