/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "schema_reader.h"

static int parse_body(schema_reader_t *r)
{
    enum bridge_kind kind = r->source->kind;

    if (kind != BRIDGE_STRUCT)
        return (schema_parse_constants(r, kind == BRIDGE_BITMASK));
    if (!schema_is(r, "{"))
        return (schema_refuse(r, "begins with '%s' and not with a brace"));
    return (schema_parse_struct(r));
}

static int check_end(schema_reader_t *r)
{
    schema_read_next(r);
    if (r->word[0])
        return (schema_refuse(r, "goes on after its closing brace, with "
            "'%s'"));
    return (0);
}

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
