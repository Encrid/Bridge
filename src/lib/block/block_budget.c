/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "block.h"

static int is_unset(bridge_world_t *w, const char *key, uint64_t value)
{
    if (value)
        return (0);
    say_error(w, "[budget] %s is unset", key);
    return (1);
}

static int is_miscounted(bridge_world_t *w, const char *key, int64_t value)
{
    if (value == BRIDGE_NONE || (value > 0 && value <= UINT32_MAX))
        return (0);
    if (!value) {
        say_error(w, "[budget] %s is unset", key);
        return (1);
    }
    say_error(w, "[budget] %s is %lld, which is neither a count nor none",
        key, (long long)value);
    return (1);
}

static int is_too_many(bridge_world_t *w, const bridge_budget_t *budget)
{
    uint64_t sets = (uint64_t)budget->sparse_sets;
    uint64_t sum;

    if (budget->sparse_sets == BRIDGE_NONE)
        sets = 0;
    sum = (uint64_t)budget->tables + sets + budget->types + 1;
    if (((uint64_t)budget->types + 63) / 64 * 64 > UINT32_MAX) {
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

static int has_unset(bridge_world_t *w, const bridge_budget_t *budget)
{
    if (is_unset(w, "entities", budget->entities))
        return (1);
    if (is_unset(w, "persistent", budget->persistent))
        return (1);
    if (is_unset(w, "columns", budget->columns))
        return (1);
    if (is_unset(w, "modules", budget->modules))
        return (1);
    if (is_unset(w, "tables", budget->tables))
        return (1);
    if (is_unset(w, "types", budget->types))
        return (1);
    if (is_unset(w, "systems", budget->systems))
        return (1);
    return (is_unset(w, "terms", budget->terms));
}

int block_check_budget(bridge_world_t *w, const bridge_budget_t *budget)
{
    if (has_unset(w, budget))
        return (1);
    if (is_miscounted(w, "sparse_sets", budget->sparse_sets))
        return (1);
    if (is_miscounted(w, "direct_indexed", budget->direct_indexed))
        return (1);
    return (is_too_many(w, budget));
}
