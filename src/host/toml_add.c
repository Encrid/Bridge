/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "toml.h"

bool toml_has_prefix(const entry_t *e, const char *const *names,
    uint32_t depth)
{
    if (e->depth < depth)
        return (false);
    for (uint32_t i = 0; i < depth; i++)
        if (strcmp(e->key[i], names[i]))
            return (false);
    return (true);
}

static const char *get_clash(const entry_t *e, char *const *key,
    uint32_t depth, bool table)
{
    uint32_t least = e->depth < depth ? e->depth : depth;

    if (!toml_has_prefix(e, (const char *const *)key, least))
        return (nullptr);
    if (e->depth == depth)
        return ("a key stated twice");
    if ((e->depth < depth && e->value.kind) || (depth < e->depth && !table))
        return ("a key that is a value and a table");
    return (nullptr);
}

static char **whole_key(const reading_t *r, char **key, uint32_t depth)
{
    char **whole = malloc(sizeof(*whole) * (r->table_depth + depth));

    if (!whole)
        return (nullptr);
    memcpy(whole, r->table, sizeof(*whole) * r->table_depth);
    memcpy(whole + r->table_depth, key, sizeof(*whole) * depth);
    return (whole);
}

bool toml_add(reading_t *r, char **key, uint32_t depth, const value_t *v)
{
    toml_t *t = r->into;
    uint32_t all = r->table_depth + depth;
    char **whole = whole_key(r, key, depth);
    const char *why = whole ? nullptr : "no memory left";
    entry_t *more = nullptr;

    for (uint32_t i = 0; i < t->count && !why; i++)
        why = get_clash(&t->entries[i], whole, all, !v->kind);
    if (!why)
        more = realloc(t->entries, sizeof(*more) * (t->count + 1));
    if (why || !more)
        return (toml_refuse(r, why ? why : "no memory left"));
    t->entries = more;
    t->entries[t->count++] = (entry_t){ whole, all, r->start, *v };
    return (true);
}
