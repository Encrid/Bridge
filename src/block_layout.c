/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/block_layout.c -- where each part of the shared block begins: the
** entity index, the sparse sets' indexes, the direct-indexed bits, the
** persistent arena, the columns.
**
** Each amount of the budget is under 2^32 by the time this runs, so none of
** these sums can leave 64 bits.
**
** storage.md, *Storage layouts*
*/

#include "block.h"

/*
** lay_sparse
**
** Places the sparse sets' indexes after the entity index. A sparse set's
** index is a u32 for every entity, and there is one for each there may be.
**
** budgets.md, *Budgets*
*/
static int lay_sparse(bridge_t *b, const bridge_budget_t *budget,
    block_layout_t *layout)
{
    uint64_t entities = budget->entities;
    uint64_t index = layout->index + sizeof(bridge_entity_t) * entities;

    layout->sets = 0;
    if (budget->sparse_sets != BRIDGE_NONE)
        layout->sets = (uint64_t)budget->sparse_sets;
    layout->sparse = (index + 7u) & ~7ull;
    if (layout->sets <= UINT32_MAX / (4 * entities))
        return (0);
    say_error(b, "[budget] cannot be satisfied: sparse_sets of %llu, with "
        "four bytes for each of %llu entities, is more than an offset of 32 "
        "bits can reach", (unsigned long long)layout->sets,
        (unsigned long long)entities);
    return (1);
}

/*
** lay_direct
**
** Places the direct-indexed bits after the sparse sets' indexes. A
** direct-indexed component's bits are one for every entity, in 64-bit words.
**
** budgets.md, *Budgets*
*/
static int lay_direct(bridge_t *b, const bridge_budget_t *budget,
    block_layout_t *layout)
{
    uint64_t entities = budget->entities;
    uint64_t indexes = 4 * entities * layout->sets;

    layout->cells = 0;
    if (budget->direct_indexed != BRIDGE_NONE)
        layout->cells = (uint64_t)budget->direct_indexed;
    layout->direct = (layout->sparse + indexes + 7u) & ~7ull;
    if (layout->cells <= UINT32_MAX / (entities / 8 + 8))
        return (0);
    say_error(b, "[budget] cannot be satisfied: direct_indexed of %llu, "
        "with a bit for each of %llu entities, is more than an offset of 32 "
        "bits can reach", (unsigned long long)layout->cells,
        (unsigned long long)entities);
    return (1);
}

/*
** lay_rest
**
** Places the arena and the columns, and sizes the block: a whole number of
** what every backend that maps it needs, and together never more than an
** offset can reach.
**
** backend.md, *Memory is adopted, not created*
*/
static int lay_rest(bridge_t *b, const bridge_budget_t *budget,
    block_layout_t *layout)
{
    uint64_t bits = 8 * ((budget->entities + 63) / 64) * layout->cells;
    uint64_t end;

    layout->arena = (layout->direct + bits + 7u) & ~7ull;
    layout->columns = (layout->arena + budget->persistent + 15u) & ~15ull;
    end = layout->columns + budget->columns + b->granule - 1;
    layout->len = end / b->granule * b->granule;
    if (layout->len <= (uint64_t)1 << 32)
        return (0);
    say_error(b, "[budget] cannot be satisfied: the shared memory would be "
        "%llu bytes and an offset into it is 32 bits",
        (unsigned long long)layout->len);
    return (1);
}

/*
** block_lay_out
**
** Works out where each part of the shared block begins, and how much
** address space it is reserved in: the largest any backend stated, and its
** guard after it.
*/
int block_lay_out(bridge_t *b, const bridge_budget_t *budget,
    block_layout_t *layout)
{
    layout->index = 8;
    if (lay_sparse(b, budget, layout) || lay_direct(b, budget, layout))
        return (1);
    if (lay_rest(b, budget, layout))
        return (1);
    layout->reserve = b->guard + b->reservation;
    if (layout->len > b->reservation)
        layout->reserve = b->guard + layout->len;
    return (0);
}
