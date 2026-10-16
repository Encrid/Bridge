/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "schema_reader.h"

static int add_field(schema_reader_t *r, schema_field_t *field)
{
    schema_t *out = r->out;
    uint32_t room = r->room ? r->room * 2 : 8;
    schema_field_t *more = out->fields;

    if (out->field_count == r->room) {
        more = realloc(out->fields, room * sizeof(*more));
        if (!more)
            return (schema_refuse(r, "ran out of memory at '%s'"));
        out->fields = more;
        r->room = room;
    }
    field->name = schema_copy_text(r->word);
    if (!field->name)
        return (schema_refuse(r, "ran out of memory at '%s'"));
    out->fields[out->field_count] = *field;
    out->field_count++;
    return (0);
}

static int read_length(schema_reader_t *r, schema_laid_t *laid)
{
    schema_field_t *field = &r->out->fields[r->out->field_count - 1];
    int64_t count;
    bool below;

    schema_read_next(r);
    if (!schema_get_integer(r, &count, &below) || below || count < 1
        || count > UINT32_MAX / laid->size)
        return (schema_refuse(r, "gives '%s' as the length of an array"));
    field->count = (uint32_t)count;
    laid->size *= (uint32_t)count;
    schema_read_next(r);
    if (!schema_is(r, "]"))
        return (schema_refuse(r, "says '%s' where an array's length ends"));
    schema_read_next(r);
    return (0);
}

static int place_field(schema_reader_t *r, const schema_laid_t *laid)
{
    schema_t *out = r->out;
    schema_field_t *field = &out->fields[out->field_count - 1];

    out->size = (out->size + laid->align - 1) & ~(laid->align - 1);
    field->offset = out->size;
    if (laid->size > UINT32_MAX - out->size - 8)
        return (schema_refuse(r, "is more bytes than can be counted, at "
            "'%s'"));
    out->size += laid->size;
    if (laid->align > out->align)
        out->align = laid->align;
    return (0);
}

static int read_field(schema_reader_t *r)
{
    schema_laid_t laid = { .field = { .count = 1 } };

    if (schema_set_type(r, &laid))
        return (1);
    schema_read_next(r);
    if (!schema_is_name(r))
        return (schema_refuse(r, "says '%s' where a field's name belongs"));
    if (add_field(r, &laid.field))
        return (1);
    schema_read_next(r);
    if (schema_is(r, "[") && read_length(r, &laid))
        return (1);
    if (!schema_is(r, ";"))
        return (schema_refuse(r, "says '%s' where a field ends"));
    return (place_field(r, &laid));
}

int schema_parse_struct(schema_reader_t *r)
{
    schema_t *out = r->out;
    int refused = 0;

    schema_read_next(r);
    while (!refused && !schema_is(r, "}")) {
        refused = read_field(r);
        if (!refused)
            schema_read_next(r);
    }
    if (refused)
        return (1);
    if (!out->field_count)
        return (schema_refuse(r, "has no field"));
    out->size = (out->size + out->align - 1) & ~(out->align - 1);
    return (0);
}
