/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/block_budget.c -- a deployment's budget, checked key by key before any
** sum is made of it.
**
** budgets.md, *Budgets*
*/

#include "block.h"

static int is_unset(bridge_t *b, const char *key, uint64_t value)
{
    if (value)
        return (0);
    say_error(b, "[budget] %s is unset", key);
    return (1);
}

/*
** is_beyond
**
** Refuses a size an offset cannot reach, by name: an offset is 32 bits.
*/
static int is_beyond(bridge_t *b, const char *key, uint64_t value)
{
    if (value <= UINT32_MAX)
        return (0);
    say_error(b, "[budget] %s of %llu is more than an offset of 32 bits can "
        "reach", key, (unsigned long long)value);
    return (1);
}

/*
** is_miscounted
**
** Refuses a count that may be none and is neither: zero is unset,
** BRIDGE_NONE is none, and nothing else below one is anything.
*/
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

/*
** has_unset
**
** Refuses a budget that leaves one of its eight amounts unset, at the first.
*/
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

/*
** block_check_budget
**
** Refuses a budget with a key unset, a size an offset of 32 bits cannot
** reach, or a count that is neither a count nor none.
*/
int block_check_budget(bridge_t *b, const bridge_budget_t *budget)
{
    if (has_unset(b, budget))
        return (1);
    if (is_beyond(b, "entities", budget->entities))
        return (1);
    if (is_beyond(b, "persistent", budget->persistent))
        return (1);
    if (is_beyond(b, "columns", budget->columns))
        return (1);
    if (is_miscounted(b, "sparse_sets", budget->sparse_sets))
        return (1);
    return (is_miscounted(b, "direct_indexed", budget->direct_indexed));
}
