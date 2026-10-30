/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bits.h"
#include "block.h"

static uint64_t get_sets(const bridge_budget_t *budget)
{
    if (budget->sparse_sets == BRIDGE_NONE)
        return (0);
    return ((uint64_t)budget->sparse_sets);
}

static int is_too_many(bridge_world_t *w, const bridge_budget_t *budget)
{
    uint64_t sum = budget->tables + get_sets(budget) + budget->types + 1;

    if ((uint64_t)bits_words(budget->types) * 64 > UINT32_MAX) {
        say_error(w, "[budget] types of %u, rounded up to a multiple"
            " of 64, is more than a count of 32 bits holds", budget->types);
        return (1);
    }
    if (sum <= UINT32_MAX)
        return (0);
    say_error(w, "[budget] tables, sparse_sets and types come to %llu, "
        "which is more than a count of 32 bits holds",
        (unsigned long long)sum);
    return (1);
}

static int is_past_hash(bridge_world_t *w, const bridge_budget_t *budget)
{
    uint64_t ids = budget->tables + get_sets(budget) + 1;
    uint64_t most = (uint64_t)STORAGE_HASH_MOST * w->table_hash_fill / 100;

    if (ids <= most)
        return (0);
    say_error(w, "[budget] tables and sparse_sets come to %llu table ids, "
        "and a table_hash_fill of %u holds at most %llu",
        (unsigned long long)ids, w->table_hash_fill,
        (unsigned long long)most);
    return (1);
}

int block_check_counts(bridge_world_t *w, const bridge_budget_t *budget)
{
    return (is_too_many(w, budget) || is_past_hash(w, budget));
}
