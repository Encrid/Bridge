/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/schema.c -- a type from its text: read whole, or refused whole.
*/

#include <stdlib.h>

#include "schema_reader.h"

/*
** parse_body
**
** Reads what the kind of the type says its text holds: constants, or the
** fields of a struct between braces.
*/
static int parse_body(schema_reader_t *r)
{
    enum bridge_kind kind = r->source->kind;

    if (kind != BRIDGE_STRUCT)
        return (schema_parse_constants(r, kind == BRIDGE_BITMASK));
    if (!schema_is(r, "{"))
        return (schema_refuse(r, "begins with '%s' and not with a brace"));
    return (schema_parse_struct(r));
}

/*
** check_end
**
** Refuses a text that goes on after its closing brace.
*/
static int check_end(schema_reader_t *r)
{
    schema_read_next(r);
    if (r->word[0])
        return (schema_refuse(r, "goes on after its closing brace, with "
            "'%s'"));
    return (0);
}

/*
** schema_parse
**
** Lays a type out from its text. 0 and `out`, or 1 and the source's `why`,
** with nothing left allocated.
*/
int schema_parse(const schema_source_t *source, schema_t *out)
{
    schema_reader_t r = { .at = source->text, .source = source, .out = out };
    int refused;

    r.value = -1;
    r.below = true;
    *out = (schema_t){};
    schema_read_next(&r);
    refused = parse_body(&r);
    if (!refused)
        refused = check_end(&r);
    if (refused)
        schema_free(out);
    return (refused);
}

/*
** schema_free
**
** Frees what a parsed type holds, and leaves it empty.
*/
void schema_free(schema_t *s)
{
    uint32_t i = 0;

    while (i < s->field_count) {
        free((char *)s->fields[i].name);
        i++;
    }
    i = 0;
    while (i < s->constant_count) {
        free(s->constants[i].name);
        i++;
    }
    free(s->fields);
    free(s->constants);
    *s = (schema_t){};
}
