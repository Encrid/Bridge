/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "module.h"

static bool is_in_share(const module_pointing_t *p, const uint8_t *bytes)
{
    const range_t *own = p->owned;
    bridge_text_t text;
    uint64_t end;
    uint32_t o = 0;

    memcpy(&text, bytes, sizeof(text));
    end = (uint64_t)text.at + text.len;
    while (text.len && o < p->own_count) {
        if (text.at < own[o].at + own[o].len && own[o].at < end)
            return (true);
        o++;
    }
    return (false);
}

static const uint8_t *get_element(const schema_field_t *f,
    const uint8_t *bytes, uint32_t i)
{
    const declared_type_t *held = f->type;
    size_t size = sizeof(bridge_text_t);

    if (f->kind != SCHEMA_TEXT && f->kind != SCHEMA_STRUCT)
        return (nullptr);
    if (i >= f->count)
        return (nullptr);
    if (f->kind == SCHEMA_STRUCT)
        size = held->size;
    return (bytes + f->offset + i * size);
}

static bool points_in(const module_pointing_t *p, const declared_type_t *c,
    const uint8_t *bytes)
{
    const schema_field_t *f = c->s.fields;
    const uint8_t *at;
    bool found = false;
    uint32_t k = 0;
    uint32_t i = 0;

    while (!found && k < c->s.field_count) {
        at = get_element(&f[k], bytes, i);
        if (at && f[k].kind == SCHEMA_STRUCT)
            found = points_in(p, f[k].type, at);
        if (at && f[k].kind == SCHEMA_TEXT)
            found = is_in_share(p, at);
        i = at ? i + 1 : 0;
        k += !at;
    }
    return (found);
}

static bool count_row(void *ctx, const uint8_t *row)
{
    module_pointing_t *p = ctx;

    p->rows += points_in(p, p->c, row);
    return (false);
}

uint32_t module_count_pointing(const bridge_t *b, const range_t *owned,
    uint32_t own_count, const declared_type_t *c)
{
    module_pointing_t p = { owned, own_count, c, 0 };

    if (c->kind == BRIDGE_STRUCT)
        (void)storage_has_any(b->storage, c->type, count_row, &p);
    return (p.rows);
}
