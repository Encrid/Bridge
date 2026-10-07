/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/block_column.c -- the column region, carved: what storage asks for is
** what it gets, to sixteen bytes.
**
** Sixteen bytes is where a column has to begin to be read at full speed. A
** block begins with its entity ids, and they begin where a column may.
**
** storage.md, *A table*
*/

#include <string.h>

#include "block.h"

constexpr uint32_t GRAIN = 16;

static uint32_t round_up(uint32_t size)
{
    return ((size + GRAIN - 1) & ~(GRAIN - 1));
}

/*
** block_column_free
**
** Gives `size` bytes of the column region back.
*/
void block_column_free(bridge_t *b, void *ptr, uint32_t size)
{
    uint32_t at = (uint32_t)((uint8_t *)ptr - b->shared) - b->columns;

    region_give(&b->column_space, at, round_up(size));
}

/*
** resize_in_place
**
** Makes a piece `want` bytes where it stands: what a shorter one no longer
** needs is given back, and a longer one grows when what follows it is free,
** which is mostly so. false if it has to move.
*/
static bool resize_in_place(bridge_t *b, void *ptr, uint32_t had,
    uint32_t want)
{
    uint32_t at = (uint32_t)((uint8_t *)ptr - b->shared) - b->columns;

    if (want < had)
        region_give(&b->column_space, at + want, had - want);
    if (want <= had)
        return (true);
    return (region_grow(&b->column_space, at, had, want));
}

/*
** block_column_regrow
**
** Answers `new_size` bytes of the column region holding what the `old_size`
** at `ptr` held, or fresh if `ptr` is null.
*/
void *block_column_regrow(bridge_t *b, void *ptr, uint32_t old_size,
    uint32_t new_size)
{
    uint8_t *base = b->shared + b->columns;
    uint32_t had = round_up(old_size);
    uint32_t want = round_up(new_size);
    uint32_t to;

    if (ptr && resize_in_place(b, ptr, had, want))
        return (ptr);
    if (!region_take(&b->column_space, want, &to))
        return (nullptr);
    if (ptr) {
        memcpy(base + to, ptr, had);
        region_give(&b->column_space, (uint32_t)((uint8_t *)ptr - base), had);
    }
    return (base + to);
}
