/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schema_reader.h"

const schema_given_t SCHEMA_GIVEN[SCHEMA_GIVEN_COUNT] = {
    { "bool", SCHEMA_BOOL, 1 },
    { "char", SCHEMA_CHAR, 1 },
    { "int8_t", SCHEMA_I8, 1 },
    { "uint8_t", SCHEMA_U8, 1 },
    { "int16_t", SCHEMA_I16, 2 },
    { "uint16_t", SCHEMA_U16, 2 },
    { "int32_t", SCHEMA_I32, 4 },
    { "uint32_t", SCHEMA_U32, 4 },
    { "int64_t", SCHEMA_I64, 8 },
    { "uint64_t", SCHEMA_U64, 8 },
    { "float", SCHEMA_F32, 4 },
    { "double", SCHEMA_F64, 8 }
};

static int set_base(schema_reader_t *r, bool bits)
{
    int given = schema_find_given(r);
    enum schema_kind kind = given < 0 ? 0 : SCHEMA_GIVEN[given].kind;

    if (kind < SCHEMA_I8 || kind > SCHEMA_U64 || kind == SCHEMA_I64
        || (kind == SCHEMA_U64 && !bits))
        return (schema_refuse(r, "begins with '%s' and not with the integer "
            "it is"));
    if (bits && (kind == SCHEMA_I8 || kind == SCHEMA_I16
        || kind == SCHEMA_I32))
        return (schema_refuse(r, "begins with '%s', and a bitmask is an "
            "unsigned integer"));
    r->out->base = kind;
    r->out->size = SCHEMA_GIVEN[given].size;
    r->out->align = SCHEMA_GIVEN[given].size;
    return (0);
}

static int add_constant(schema_reader_t *r)
{
    schema_t *out = r->out;

    out->constants[out->constant_count].name = schema_copy_text(r->word);
    out->constant_count++;
    if (!out->constants[out->constant_count - 1].name)
        return (schema_refuse(r, "ran out of memory at '%s'"));
    return (0);
}

static int read_constant(schema_reader_t *r, bool bits)
{
    schema_t *out = r->out;

    if (!schema_is_name(r))
        return (schema_refuse(r, "says '%s' where a constant's name "
            "belongs"));
    if (add_constant(r))
        return (1);
    schema_read_next(r);
    if (schema_set_value(r, bits))
        return (1);
    out->constants[out->constant_count - 1].value = r->value;
    if (schema_is(r, ",")) {
        schema_read_next(r);
        return (0);
    }
    if (!schema_is(r, "}"))
        return (schema_refuse(r, "says '%s' where a constant ends"));
    return (0);
}

int schema_parse_constants(schema_reader_t *r, bool bits)
{
    int refused = set_base(r, bits);

    if (refused)
        return (1);
    schema_read_next(r);
    if (!schema_is(r, "{"))
        return (schema_refuse(r, "says '%s' where its brace belongs"));
    r->out->constants = schema_take_room(r, ',', sizeof(*r->out->constants));
    if (!r->out->constants)
        return (schema_refuse(r, "ran out of memory at '%s'"));
    schema_read_next(r);
    while (!refused && !schema_is(r, "}"))
        refused = read_constant(r, bits);
    if (refused)
        return (1);
    if (!r->out->constant_count)
        return (schema_refuse(r, "has no constant"));
    return (0);
}
