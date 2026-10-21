/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "region.h"

bool region_make_room(region_t *r, uint32_t ranges)
{
    range_t *gaps;

    if (ranges + 1 <= r->room)
        return (true);
    gaps = realloc(r->gaps, ((size_t)ranges + 1) * sizeof(*gaps));
    if (!gaps)
        return (false);
    r->gaps = gaps;
    r->room = ranges + 1;
    return (true);
}

void region_drop_gap(region_t *r, uint32_t i)
{
    size_t after = (r->count - i - 1) * sizeof(*r->gaps);

    memmove(&r->gaps[i], &r->gaps[i + 1], after);
    r->count--;
}

static uint32_t add_gap(region_t *r, uint32_t i, range_t piece)
{
    range_t *gaps = r->gaps;

    if (i && gaps[i - 1].at + gaps[i - 1].len == piece.at) {
        gaps[i - 1].len += piece.len;
        return (i - 1);
    }
    if (r->count == r->room)
        return (r->count);
    memmove(&gaps[i + 1], &gaps[i], (r->count - i) * sizeof(*gaps));
    gaps[i] = piece;
    r->count++;
    return (i);
}

void region_give(region_t *r, uint32_t at, uint32_t len)
{
    range_t *gaps = r->gaps;
    uint32_t i = 0;

    while (i < r->count && gaps[i].at < at)
        i++;
    i = add_gap(r, i, (range_t){ at, len });
    if (i == r->count)
        return;
    if (i + 1 < r->count && gaps[i].at + gaps[i].len == gaps[i + 1].at) {
        gaps[i].len += gaps[i + 1].len;
        region_drop_gap(r, i + 1);
    }
    if (gaps[i].at + gaps[i].len == r->used) {
        r->used = gaps[i].at;
        region_drop_gap(r, i);
    }
}

void region_bump(region_t *r, uint32_t by)
{
    r->used += by;
    if (r->used > r->peak)
        r->peak = r->used;
}
