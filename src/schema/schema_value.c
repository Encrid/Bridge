/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schema_reader.h"

bool schema_fits(enum schema_kind base, int64_t value, bool below)
{
    uint64_t bits = (uint64_t)value;

    switch (base) {
        case SCHEMA_I8:
            return (below ? value >= INT8_MIN : bits <= INT8_MAX);
        case SCHEMA_I16:
            return (below ? value >= INT16_MIN : bits <= INT16_MAX);
        case SCHEMA_I32:
            return (below ? value >= INT32_MIN : bits <= INT32_MAX);
        case SCHEMA_U8:
            return (!below && bits <= UINT8_MAX);
        case SCHEMA_U16:
            return (!below && bits <= UINT16_MAX);
        case SCHEMA_U32:
            return (!below && bits <= UINT32_MAX);
        default:
            return (!below);
    }
}

static int read_value(schema_reader_t *r)
{
    schema_read_next(r);
    if (!schema_get_integer(r, &r->value, &r->below)
        || !schema_fits(r->out->base, r->value, r->below))
        return (schema_refuse(r, "gives a constant the value '%s', which "
            "the integer it is cannot hold"));
    schema_read_next(r);
    return (0);
}

int schema_set_value(schema_reader_t *r, bool bits)
{
    if (schema_is(r, "="))
        return (read_value(r));
    if (bits)
        return (schema_refuse(r, "says '%s' where a bit's value belongs: a "
            "bitmask states every one"));
    r->value++;
    r->below = r->value < 0;
    if (!schema_fits(r->out->base, r->value, r->below))
        return (schema_refuse(r, "counts up to a constant before '%s' that "
            "the integer it is cannot hold"));
    return (0);
}
