/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "config.h"

static const config_seeder_fn_t CONFIG_SEEDERS[SCHEMA_STRUCT + 1] = {
    [SCHEMA_BOOL] = config_seed_bool,
    [SCHEMA_CHAR] = config_seed_integer,
    [SCHEMA_I8] = config_seed_integer,
    [SCHEMA_U8] = config_seed_integer,
    [SCHEMA_I16] = config_seed_integer,
    [SCHEMA_U16] = config_seed_integer,
    [SCHEMA_I32] = config_seed_integer,
    [SCHEMA_U32] = config_seed_integer,
    [SCHEMA_I64] = config_seed_integer,
    [SCHEMA_U64] = config_seed_integer,
    [SCHEMA_F32] = config_seed_float,
    [SCHEMA_F64] = config_seed_float,
    [SCHEMA_ENUM] = config_seed_enum,
    [SCHEMA_BITMASK] = config_seed_bitmask,
    [SCHEMA_TEXT] = config_seed_text,
    [SCHEMA_STRUCT] = config_seed_struct
};

static bool is_repeated(const bridge_setting_t *s, uint32_t i)
{
    for (uint32_t j = 0; j < i; j++) {
        if (!strcmp(s[j].key, s[i].key))
            return (true);
    }
    return (false);
}

int config_check_settings(config_seed_t *seed)
{
    const bridge_setting_t *s = seed->cfg->settings;

    for (uint32_t i = 0; i < seed->cfg->setting_count; i++) {
        if (!s[i].key)
            return (config_refuse(seed, "its setting %u has no key", i));
        if (s[i].kind < BRIDGE_BOOL || s[i].kind > BRIDGE_STRING)
            return (config_refuse(seed, "its setting '%.80s' has the kind "
                "%u, which is no kind of a setting", s[i].key,
                (unsigned)s[i].kind));
        if (s[i].kind == BRIDGE_STRING && !s[i].v.s)
            return (config_refuse(seed, "its setting '%.80s' is text, "
                "and its text is null", s[i].key));
        if (is_repeated(s, i))
            return (config_refuse(seed, "its setting '%.80s' is handed "
                "more than once", s[i].key));
    }
    return (0);
}

static const schema_field_t *find_field(const config_seed_t *seed,
    const char *key)
{
    const schema_t *s = &seed->c->s;
    uint32_t k = 0;

    while (k < s->field_count && strcmp(s->fields[k].name, key))
        k++;
    if (k == s->field_count)
        return (nullptr);
    return (&s->fields[k]);
}

static int seed_field(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s)
{
    if (field->count > 1)
        return (config_seed_array(seed, field, s));
    return (CONFIG_SEEDERS[field->kind](seed, field, s));
}

int config_seed_setting(config_seed_t *seed, const bridge_setting_t *s)
{
    const schema_field_t *field = find_field(seed, s->key);

    if (!field)
        return (config_refuse(seed, "its settings have a key '%.80s', and "
            "'%.100s' has no field of that name", s->key,
            seed->c->name));
    return (seed_field(seed, field, s));
}
