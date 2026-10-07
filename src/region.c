/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/region.c -- a region's list of gaps, and a piece given back to it.
*/

#include <stdlib.h>
#include <string.h>

#include "region.h"

/*
** region_make_room
**
** Makes room in the list for the gaps that `ranges` ranges handed out can
** leave.
*/
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

/*
** region_drop_gap
**
** Takes gap `i` out of the list.
*/
void region_drop_gap(region_t *r, uint32_t i)
{
    size_t after = (r->count - i - 1) * sizeof(*r->gaps);

    memmove(&r->gaps[i], &r->gaps[i + 1], after);
    r->count--;
}

/*
** add_gap
**
** Records a freed piece: added to the gap that ends where it begins, or a
** gap of its own at `i`, where it keeps the list in offset order. Answers
** which gap it is in, or how many there are if it could not be recorded.
**
** The column region's list is as long as there may be tables, and the
** arena's as long as the ranges modules hold, plus one: region_make_room saw
** to it when each was taken. So there is room; were there not, the gap would
** go unrecorded and its bytes unused.
*/
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

/*
** region_give
**
** Gives a piece back. It is joined to the gap after it, and a gap at the top
** is not a gap: it is space never used.
*/
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
