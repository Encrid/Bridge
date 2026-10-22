/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "config.h"

bool config_states_key(const cfile_t *f, const void *asked, said_t *said)
{
    const keyed_t *k = asked;
    const entry_t *e = nullptr;

    for (uint32_t i = 0; i < f->toml.count; i++) {
        e = &f->toml.entries[i];
        if (e->value.kind && e->depth == k->depth
            && toml_has_prefix(e, k->names, k->depth)) {
            *said = (said_t){ &e->value, 0, e->line };
            return (true);
        }
    }
    return (false);
}

static bool is_module_key(const entry_t *e, const char *table,
    const keyed_t *k)
{
    if (!e->value.kind || e->depth != k->depth + 2
        || strcmp(e->key[0], "module") || strcmp(e->key[1], table))
        return (false);
    for (uint32_t i = 0; i < k->depth; i++)
        if (strcmp(e->key[i + 2], k->names[i]))
            return (false);
    return (true);
}

static const entry_t *find_in_table(const cfile_t *f, const char *table,
    const keyed_t *k)
{
    for (uint32_t i = 0; i < f->toml.count; i++)
        if (is_module_key(&f->toml.entries[i], table, k))
            return (&f->toml.entries[i]);
    return (nullptr);
}

bool config_states_module(const cfile_t *f, const void *asked, said_t *said)
{
    const module_key_t *m = asked;
    const entry_t *e = nullptr;

    for (uint32_t t = 0; t < 3 && !e; t++)
        e = m->tables[t] ? find_in_table(f, m->tables[t], &m->key) : nullptr;
    if (e)
        *said = (said_t){ &e->value, 0, e->line };
    return (e != nullptr);
}
