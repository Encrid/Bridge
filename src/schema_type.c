/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schema_reader.h"

static enum schema_kind get_field_kind(enum bridge_kind kind)
{
    if (kind == BRIDGE_STRUCT)
        return (SCHEMA_STRUCT);
    if (kind == BRIDGE_ENUM)
        return (SCHEMA_ENUM);
    return (SCHEMA_BITMASK);
}

static bool set_named_type(schema_reader_t *r, schema_laid_t *laid)
{
    const schema_source_t *source = r->source;
    schema_named_t named;

    if (!schema_is_name(r) || !source->find(source->ctx, r->word, &named))
        return (false);
    laid->field.kind = get_field_kind(named.kind);
    laid->field.type = named.type;
    laid->size = named.size;
    laid->align = named.align;
    return (true);
}

int schema_set_type(schema_reader_t *r, schema_laid_t *laid)
{
    int given = schema_find_given(r);

    if (given >= 0) {
        laid->field.kind = SCHEMA_GIVEN[given].kind;
        laid->size = SCHEMA_GIVEN[given].size;
        laid->align = SCHEMA_GIVEN[given].size;
        return (0);
    }
    if (schema_is(r, "bridge_text_t")) {
        laid->field.kind = SCHEMA_TEXT;
        laid->size = sizeof(bridge_text_t);
        laid->align = alignof(bridge_text_t);
        return (0);
    }
    if (set_named_type(r, laid))
        return (0);
    return (schema_refuse(r, "says '%s', which is neither a type a field "
        "can be nor one its namespace has declared"));
}
