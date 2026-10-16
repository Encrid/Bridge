/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "block.h"

static int is_beyond(bridge_t *b, const char *key, uint64_t value)
{
    if (value <= UINT32_MAX)
        return (0);
    say_error(b, "[budget] %s of %llu is more than an offset of 32 bits can "
        "reach", key, (unsigned long long)value);
    return (1);
}

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

int block_lay_out(bridge_t *b, const bridge_budget_t *budget,
    block_layout_t *layout)
{
    if (is_beyond(b, "entities", budget->entities))
        return (1);
    if (is_beyond(b, "persistent", budget->persistent))
        return (1);
    if (is_beyond(b, "columns", budget->columns))
        return (1);
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
