/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/schema_struct.c -- a struct read from its descriptor, a field at a
** time, and laid out as C lays it out.
*/

#include <stdlib.h>

#include "schema_reader.h"

/*
** add_field
**
** Adds a field named by the token to the struct, with room made for it.
**
** failure.md, *Out of memory*
*/
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

/*
** read_length
**
** Reads the length of an inline array, from its opening bracket to past its
** closing one, and makes the field just added that many.
**
** schema.md, *The type set*
*/
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

/*
** place_field
**
** Places the field just added at the next multiple of its alignment, and
** counts it in the struct's size and alignment.
**
** schema.md, *The type set*
*/
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

/*
** read_field
**
** Reads one field, from its type to its semicolon, and lays it out.
**
** schema.md, *The type set*
*/
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

/*
** schema_parse_struct
**
** Reads a struct's fields, and rounds its size to a multiple of its widest.
**
** schema.md, *The type set*
*/
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
