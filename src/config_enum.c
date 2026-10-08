/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/config_enum.c -- a setting written into a field that is an enum or a
** bitmask.
*/

#include <string.h>

#include "config.h"

/*
** find_constant
**
** Answers the value of an enum's constant, by its name. false if there is no
** name, no enum, or no constant of that name.
*/
static bool find_constant(const declared_type_t *of, const char *name,
    int64_t *value)
{
    uint32_t i = 0;

    if (!name || !of)
        return (false);
    while (i < of->s.constant_count && strcmp(of->s.constants[i].name, name))
        i++;
    if (i == of->s.constant_count)
        return (false);
    *value = of->s.constants[i].value;
    return (true);
}

/*
** fits_enum
**
** Answers whether an enum holds an integer. An enum is the integer its
** author stated: as many bytes, signed or not.
*/
static bool fits_enum(const declared_type_t *of, int64_t value)
{
    enum schema_kind base = of->s.base;
    int64_t most = INT64_MAX >> (64 - 8 * of->s.size);

    if (base == SCHEMA_I8 || base == SCHEMA_I16 || base == SCHEMA_I32)
        return (value >= -most - 1 && value <= most);
    return (value >= 0 && value <= most * 2 + 1);
}

/*
** config_seed_enum
**
** Writes an integer into an enum, or the value of the constant a text names:
** a constant's name is the one thing a deployment can be expected to know.
**
** config.md, *A module's own settings are a component*
** schema.md, *The type set*
*/
int config_seed_enum(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s)
{
    const declared_type_t *of = field->type;
    int64_t value = s->v.i;

    if (s->kind == BRIDGE_INT && !fits_enum(of, value))
        return (config_refuse(seed, "its setting '%.80s' is %lld, which "
            "the enum of the field it names cannot hold", s->key,
            (long long)value));
    if (s->kind == BRIDGE_STRING && !find_constant(of, s->v.s, &value))
        return (config_refuse(seed, "its setting '%.80s' is '%.60s', which "
            "the enum of the field it names has no constant for", s->key,
            s->v.s ? s->v.s : ""));
    if (s->kind != BRIDGE_INT && s->kind != BRIDGE_STRING)
        return (config_say_misfit(seed, s, "an enum"));
    memcpy(seed->row + field->offset, &value, of->s.size);
    return (0);
}

/*
** config_seed_bitmask
**
** Writes an integer into a bitmask that has that many bits.
*/
int config_seed_bitmask(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s)
{
    const declared_type_t *of = field->type;
    uint64_t most = UINT64_MAX >> (64 - 8 * of->s.size);

    if (s->kind != BRIDGE_INT || s->v.i < 0 || (uint64_t)s->v.i > most)
        return (config_say_misfit(seed, s, "a bitmask"));
    memcpy(seed->row + field->offset, &s->v.i, of->s.size);
    return (0);
}
