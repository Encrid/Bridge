/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "config.h"

static const char *check_named(const entry_t *e, const char *const *names,
    const enum check *checks)
{
    for (uint32_t i = 0; names[i]; i++)
        if (!strcmp(e->key[e->depth - 1], names[i]))
            return (config_check_value(&e->value, checks[i]));
    return ("no key of Bridge-Host's or Bridge's");
}

const char *config_check_world(const entry_t *e, bool local)
{
    static const char *const NAMES[] = { "phases", "threads", "rounds",
        "live", "presets", nullptr };
    static const enum check CHECKS[] = { CHECK_NAMES, CHECK_COUNT,
        CHECK_ROUNDS, CHECK_FLAG, CHECK_NAMES };

    if (e->depth != 1)
        return ("no key of Bridge-Host's or Bridge's");
    if (local)
        return ("fixed when the world is made: not in the local file");
    return (check_named(e, NAMES, CHECKS));
}

const char *config_check_budget(const entry_t *e, bool local)
{
    static const char *const NAMES[] = { "entities", "tables", "types",
        "systems", "terms", "modules", "persistent", "columns", "sparse_sets",
        "direct_indexed", nullptr };
    static const enum check CHECKS[] = { CHECK_COUNT, CHECK_COUNT,
        CHECK_COUNT, CHECK_COUNT, CHECK_COUNT, CHECK_COUNT, CHECK_COUNT,
        CHECK_COUNT, CHECK_LIMIT, CHECK_LIMIT };

    if (e->depth != 2)
        return ("no key of Bridge-Host's or Bridge's");
    if (local)
        return ("fixed when the world is made: not in the local file");
    return (check_named(e, NAMES, CHECKS));
}

const char *config_check_backend(const entry_t *e, bool local)
{
    if (e->depth != 2)
        return ("no key of Bridge-Host's or Bridge's");
    if (local)
        return ("fixed when the world is made: not in the local file");
    return (config_check_value(&e->value, CHECK_TEXT));
}

const char *config_check_content(const entry_t *e)
{
    static const char *const NAMES[] = { "paths", "ignore", nullptr };
    static const enum check CHECKS[] = { CHECK_NAMES, CHECK_NAMES };

    if (e->depth == 2)
        return (check_named(e, NAMES, CHECKS));
    if (e->depth == 3 && !strcmp(e->key[1], "load"))
        return (config_check_value(&e->value, CHECK_FLAG));
    if (e->depth == 3 && !strcmp(e->key[1], "arrive"))
        return (config_check_value(&e->value, CHECK_ARRIVE));
    return ("no key of Bridge-Host's or Bridge's");
}
