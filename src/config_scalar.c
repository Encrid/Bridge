/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "config.h"

static const config_integer_t CONFIG_INTEGERS[SCHEMA_U64 + 1] = {
    [SCHEMA_CHAR] = { INT8_MIN, INT8_MAX, 1 },
    [SCHEMA_I8] = { INT8_MIN, INT8_MAX, 1 },
    [SCHEMA_U8] = { 0, UINT8_MAX, 1 },
    [SCHEMA_I16] = { INT16_MIN, INT16_MAX, 2 },
    [SCHEMA_U16] = { 0, UINT16_MAX, 2 },
    [SCHEMA_I32] = { INT32_MIN, INT32_MAX, 4 },
    [SCHEMA_U32] = { 0, UINT32_MAX, 4 },
    [SCHEMA_I64] = { INT64_MIN, INT64_MAX, 8 },
    [SCHEMA_U64] = { 0, INT64_MAX, 8 }
};

int config_seed_bool(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s)
{
    if (s->kind != BRIDGE_BOOL)
        return (config_say_misfit(seed, s, "a bool"));
    seed->row[field->offset] = s->v.b ? 1 : 0;
    return (0);
}

int config_seed_float(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s)
{
    uint8_t *at = seed->row + field->offset;
    double wide = s->kind == BRIDGE_FLOAT ? s->v.f : (double)s->v.i;
    float narrow = (float)wide;

    if (s->kind != BRIDGE_FLOAT && s->kind != BRIDGE_INT)
        return (config_say_misfit(seed, s, "a float"));
    if (field->kind == SCHEMA_F32)
        memcpy(at, &narrow, 4);
    else
        memcpy(at, &wide, 8);
    return (0);
}

static int put_integer(uint8_t *at, enum schema_kind kind, int64_t value)
{
    const config_integer_t *range = &CONFIG_INTEGERS[kind];

    if (value < range->least || value > range->most)
        return (1);
    memcpy(at, &value, range->size);
    return (0);
}

int config_seed_integer(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s)
{
    uint8_t *at = seed->row + field->offset;

    if (s->kind != BRIDGE_INT)
        return (config_say_misfit(seed, s, "an integer"));
    if (put_integer(at, field->kind, s->v.i))
        return (config_refuse(seed, "its setting '%.80s' is %lld, which "
            "the field it names cannot hold", s->key, (long long)s->v.i));
    return (0);
}
