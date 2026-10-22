/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "config.h"

static const char *check_order(const entry_t *e)
{
    if (e->depth != 2 || strcmp(e->key[1], "before"))
        return ("no key of Bridge-Host's or Bridge's");
    return (config_check_value(&e->value, CHECK_PAIRS));
}

static const char *check_entry(const config_t *c, const entry_t *e,
    bool local)
{
    const char *top = e->key[0];

    if (!strcmp(top, "module"))
        return (config_check_module(c, e));
    if (!strcmp(top, "content"))
        return (config_check_content(e));
    if (!strcmp(top, "order"))
        return (check_order(e));
    if (!strcmp(top, "budget"))
        return (config_check_budget(e, local));
    if (!strcmp(top, "checks"))
        return (config_check_checks(e, local));
    if (!strcmp(top, "backend"))
        return (config_check_backend(e, local));
    return (config_check_world(e, local));
}

static const char *check_header(const cfile_t *f, const entry_t *table)
{
    const entry_t *e = nullptr;

    for (uint32_t i = 0; i < f->toml.count; i++) {
        e = &f->toml.entries[i];
        if (e->value.kind && e->depth > table->depth && toml_has_prefix(e,
            (const char *const *)table->key, table->depth))
            return (nullptr);
    }
    return ("a table with nothing in it");
}

static uint32_t check_file(const config_t *c, uint32_t at)
{
    const cfile_t *f = &c->files[at];
    const entry_t *e = nullptr;
    const char *why = nullptr;
    char key[256];
    uint32_t refused = 0;

    for (uint32_t i = 0; i < f->toml.count; i++) {
        e = &f->toml.entries[i];
        why = e->value.kind ? check_entry(c, e, (int32_t)at == c->local)
            : check_header(f, e);
        if (why)
            printf("refused: %s:%u: %s: %s\n", f->path, e->line,
                config_say_key((const char *const *)e->key, e->depth, key,
                sizeof(key)), why);
        refused += why != nullptr;
    }
    return (refused);
}

uint32_t config_check(config_t *c)
{
    uint32_t refused = 0;

    for (uint32_t i = 0; i < c->count; i++)
        refused += check_file(c, i);
    c->refused += refused;
    return (refused);
}
