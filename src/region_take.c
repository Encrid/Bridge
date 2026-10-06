/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/region_take.c -- a piece taken from a region, and one made longer.
*/

#include "region.h"

/*
** find_best
**
** Answers the smallest gap that holds `len` bytes, or how many gaps there
** are if none does.
*/
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

/*
** region_take
**
** Takes a piece of `len` bytes: from the smallest gap that holds it, and
** otherwise from what has never been used.
*/
int region_take(region_t *r, uint32_t len, uint32_t *at)
{
    uint32_t best = find_best(r, len);

    if (best == r->count && len > r->len - r->used)
        return (0);
    if (best == r->count) {
        *at = r->used;
        r->used += len;
        return (1);
    }
    *at = r->gaps[best].at;
    r->gaps[best].at += len;
    r->gaps[best].len -= len;
    if (!r->gaps[best].len)
        region_drop_gap(r, best);
    return (1);
}

/*
** find_after
**
** Answers the gap that begins at `end`, or how many gaps there are if none
** does.
*/
static uint32_t find_after(const region_t *r, uint32_t end)
{
    uint32_t i = 0;

    while (i < r->count && r->gaps[i].at < end)
        i++;
    if (i < r->count && r->gaps[i].at == end)
        return (i);
    return (r->count);
}

/*
** region_grow
**
** Makes the piece at `at` longer where it stands. Mostly it is the last
** piece handed out, and what follows it has never been used; otherwise the
** gap after it gives what is asked, if it is that long.
*/
bool region_grow(region_t *r, uint32_t at, uint32_t had, uint32_t want)
{
    uint32_t more = want - had;
    uint32_t i = find_after(r, at + had);

    if (at + had == r->used && more <= r->len - r->used) {
        r->used += more;
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

/*
** region_get_most
**
** Answers the longest piece the region could give now.
*/
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
