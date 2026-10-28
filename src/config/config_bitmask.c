/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "config.h"

static uint64_t get_bitmask_bits(const declared_type_t *of)
{
    uint64_t bits = 0;
    uint32_t i = 0;

    while (i < of->s.constant_count) {
        bits |= (uint64_t)of->s.constants[i].value;
        i++;
    }
    return (bits);
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
    if ((uint64_t)s->v.i & ~get_bitmask_bits(of))
        return (config_refuse(seed, "its setting '%.80s' is %lld, which has "
            "a bit that no constant of the bitmask of the field it names has",
            s->key, (long long)s->v.i));
    memcpy(seed->row + field->offset, &s->v.i, of->s.size);
    return (0);
}
