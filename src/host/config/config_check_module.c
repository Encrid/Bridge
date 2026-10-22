/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "config.h"

static const char *check_table(const config_t *c, const char *table)
{
    static const char *const BACKEND[] = { "backend" };
    const keyed_t under = { BACKEND, 1 };
    names_t backends = {};

    if (!strcmp(table, "all") || (strchr(table, ':') && strchr(table, '@')))
        return (nullptr);
    if (strchr(table, ':'))
        return ("a module is named with its version");
    backends = config_names(c, &under);
    for (uint32_t i = 0; i < backends.count; i++)
        if (!strcmp(backends.names[i], table))
            return (nullptr);
    return ("no backend of that name");
}

static const char *check_rule(const entry_t *e)
{
    for (uint32_t i = 0; i < MODULE_KEY_COUNT; i++)
        if (!strcmp(e->key[2], MODULE_KEYS[i].name))
            return (config_check_value(&e->value, MODULE_KEYS[i].check));
    return ("no key of a module");
}

static const char *check_setting(const entry_t *e)
{
    if (!strchr(e->key[1], ':'))
        return ("a setting is stated for one module, by its identity");
    if (e->depth == 4)
        return (config_check_value(&e->value, CHECK_SETTING));
    if (e->depth == 5 && !strcmp(e->key[4], "path"))
        return (config_check_value(&e->value, CHECK_TEXT));
    return ("a setting is a value, or { path = \"...\" }");
}

const char *config_check_module(const config_t *c, const entry_t *e)
{
    const char *why = nullptr;

    if (e->depth < 3)
        return ("no key of Bridge-Host's or Bridge's");
    why = check_table(c, e->key[1]);
    if (why || e->depth == 3)
        return (why ? why : check_rule(e));
    if (!strcmp(e->key[2], "config"))
        return (check_setting(e));
    if (e->depth == 4 && !strcmp(e->key[2], "phases"))
        return (config_check_value(&e->value, CHECK_TEXT));
    return ("no key of a module");
}
