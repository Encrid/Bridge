/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "region.h"

static uint32_t find_best(const region_t *r, uint32_t len)
{
    uint32_t best = r->count;
    uint32_t i = 0;

    while (i < r->count) {
        if (r->gaps[i].len >= len
            && (best == r->count || r->gaps[i].len < r->gaps[best].len))
            best = i;
        i++;
    }
    return (best);
}

int region_take(region_t *r, uint32_t len, uint32_t *at)
{
    uint32_t best = find_best(r, len);

    if (best == r->count && len > r->len - r->used)
        return (0);
    if (best == r->count) {
        *at = r->used;
        region_bump(r, len);
        return (1);
    }
    *at = r->gaps[best].at;
    r->gaps[best].at += len;
    r->gaps[best].len -= len;
    if (!r->gaps[best].len)
        region_drop_gap(r, best);
    return (1);
}

static uint32_t find_after(const region_t *r, uint32_t end)
{
    uint32_t i = 0;

    while (i < r->count && r->gaps[i].at < end)
        i++;
    if (i < r->count && r->gaps[i].at == end)
        return (i);
    return (r->count);
}

bool region_grow(region_t *r, uint32_t at, uint32_t had, uint32_t want)
{
    uint32_t more = want - had;
    uint32_t i = find_after(r, at + had);

    if (at + had == r->used && more <= r->len - r->used) {
        region_bump(r, more);
        return (true);
    }
    if (i == r->count || r->gaps[i].len < more)
        return (false);
    r->gaps[i].at += more;
    r->gaps[i].len -= more;
    if (!r->gaps[i].len)
        region_drop_gap(r, i);
    return (true);
}

uint32_t region_get_most(const region_t *r)
{
    uint32_t most = r->len - r->used;
    uint32_t i = 0;

    while (i < r->count) {
        if (r->gaps[i].len > most)
            most = r->gaps[i].len;
        i++;
    }
    return (most);
}
