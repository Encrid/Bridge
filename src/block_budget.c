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
    return (is_miscounted(b, "direct_indexed", budget->direct_indexed));
}
