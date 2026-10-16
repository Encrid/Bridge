/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "config.h"

static bool find_constant(const declared_type_t *of, const char *name,
    int64_t *value)
{
    uint32_t i = 0;

    while (i < of->s.constant_count && strcmp(of->s.constants[i].name, name))
        i++;
    if (i == of->s.constant_count)
        return (false);
    *value = of->s.constants[i].value;
    return (true);
}

static bool fits_enum(const declared_type_t *of, int64_t value)
{
    enum schema_kind base = of->s.base;
    int64_t most = INT64_MAX >> (64 - 8 * of->s.size);

    if (base == SCHEMA_I8 || base == SCHEMA_I16 || base == SCHEMA_I32)
        return (value >= -most - 1 && value <= most);
    return (value >= 0 && value <= most * 2 + 1);
}

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
            s->v.s));
    if (s->kind != BRIDGE_INT && s->kind != BRIDGE_STRING)
        return (config_say_misfit(seed, s, "an enum"));
    memcpy(seed->row + field->offset, &value, of->s.size);
    return (0);
}

int config_seed_bitmask(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s)
{
    const declared_type_t *of = field->type;
    uint64_t most = UINT64_MAX >> (64 - 8 * of->s.size);

    if (s->kind != BRIDGE_INT)
        return (config_say_misfit(seed, s, "a bitmask"));
    if (s->v.i < 0 || (uint64_t)s->v.i > most)
        return (config_refuse(seed, "its setting '%.80s' is %lld, which "
            "the bitmask of the field it names cannot hold", s->key,
            (long long)s->v.i));
    memcpy(seed->row + field->offset, &s->v.i, of->s.size);
    return (0);
}
