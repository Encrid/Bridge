/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "block.h"

static int is_unset(bridge_t *b, const char *key, uint64_t value)
{
    if (value)
        return (0);
    say_error(b, "[budget] %s is unset", key);
    return (1);
}

static int is_miscounted(bridge_t *b, const char *key, int64_t value)
{
    if (value == BRIDGE_NONE || (value > 0 && value <= UINT32_MAX))
        return (0);
    if (!value) {
        say_error(b, "[budget] %s is unset", key);
        return (1);
    }
    say_error(b, "[budget] %s is %lld, which is neither a count nor none",
        key, (long long)value);
    return (1);
}

static int is_too_many(bridge_t *b, const bridge_budget_t *budget)
{
    uint64_t sets = (uint64_t)budget->sparse_sets;
    uint64_t sum;

    if (budget->sparse_sets == BRIDGE_NONE)
        sets = 0;
    sum = (uint64_t)budget->tables + sets + budget->types + 1;
    if (((uint64_t)budget->types + 63) / 64 * 64 > UINT32_MAX) {
        say_error(b, "[budget] types of %u, rounded up to a multiple"
            " of 64, is more than a count of 32 bits holds", budget->types);
        return (1);
    }
    if (sum <= UINT32_MAX)
        return (0);
    say_error(b, "[budget] tables, sparse_sets and types come to %llu, "
        "which is more than a count of 32 bits holds",
        (unsigned long long)sum);
    return (1);
}

static int has_unset(bridge_t *b, const bridge_budget_t *budget)
{
    if (is_unset(b, "entities", budget->entities))
        return (1);
    if (is_unset(b, "persistent", budget->persistent))
        return (1);
    if (is_unset(b, "columns", budget->columns))
        return (1);
    if (is_unset(b, "modules", budget->modules))
        return (1);
    if (is_unset(b, "tables", budget->tables))
        return (1);
    if (is_unset(b, "types", budget->types))
        return (1);
    if (is_unset(b, "systems", budget->systems))
        return (1);
    return (is_unset(b, "terms", budget->terms));
}

int block_check_budget(bridge_t *b, const bridge_budget_t *budget)
{
    if (has_unset(b, budget))
        return (1);
    if (is_miscounted(b, "sparse_sets", budget->sparse_sets))
        return (1);
    if (is_miscounted(b, "direct_indexed", budget->direct_indexed))
        return (1);
    return (is_too_many(b, budget));
}
