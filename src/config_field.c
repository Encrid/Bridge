/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/config_field.c -- a setting written into the field its key names, by
** what the field is.
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

/*
** find_field
**
** Answers the field of the config component that a key names, or nullptr.
*/
static const schema_field_t *find_field(const config_seed_t *seed,
    const char *key)
{
    const schema_t *s = &seed->c->s;
    uint32_t k = 0;

    if (!key)
        return (nullptr);
    while (k < s->field_count && strcmp(s->fields[k].name, key))
        k++;
    if (k == s->field_count)
        return (nullptr);
    return (&s->fields[k]);
}

/*
** seed_field
**
** Writes a setting into a field: an inline array by itself, and any other by
** the seeder of its kind.
*/
static int seed_field(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s)
{
    if (field->count > 1)
        return (config_seed_array(seed, field, s));
    if (field->kind > SCHEMA_STRUCT || !CONFIG_SEEDERS[field->kind])
        return (config_say_misfit(seed, s, "of a type no setting fills"));
    return (CONFIG_SEEDERS[field->kind](seed, field, s));
}

/*
** config_seed_setting
**
** Writes one setting into the field its key names.
**
** config.md, *A module's own settings are a component*
*/
int config_seed_setting(config_seed_t *seed, const bridge_setting_t *s)
{
    const schema_field_t *field = find_field(seed, s->key);

    if (!field)
        return (config_refuse(seed, "its settings have a key '%.80s', and "
            "'%.100s' has no field of that name", s->key ? s->key : "",
            seed->c->name));
    return (seed_field(seed, field, s));
}
