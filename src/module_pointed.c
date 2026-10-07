/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/module_pointed.c -- whether a row holds a string in a module's share
** of the persistent arena. A string a component keeps is a bridge_text_t
** field, so every reference there is can be found: in a field, in an array
** of them, or in a struct a field holds.
**
** strings.md, *It is given back when nothing points into it*
*/

#include <string.h>

#include "module.h"

/*
** is_in_share
**
** Answers whether the text at `bytes` lies in a module's share.
*/
static bool is_in_share(const module_t *m, const uint8_t *bytes)
{
    const range_t *own = m->owned;
    bridge_text_t text;
    uint64_t end;
    uint32_t o = 0;

    memcpy(&text, bytes, sizeof(text));
    end = (uint64_t)text.at + text.len;
    while (text.len && o < m->own_count) {
        if (text.at < own[o].at + own[o].len && own[o].at < end)
            return (true);
        o++;
    }
    return (false);
}

/*
** get_element
**
** Answers where element `i` of a field is in a value, or nullptr if the
** field has no such element or is of a kind that holds no string: only a
** text does, and a struct may.
*/
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

/*
** points_in
**
** Answers whether a value holds a string in a module's share: every element
** of every field is read, one at a time, and a struct as a value of its
** own.
*/
static bool points_in(const module_t *m, const declared_type_t *c,
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
            found = points_in(m, f[k].type, at);
        if (at && f[k].kind == SCHEMA_TEXT)
            found = is_in_share(m, at);
        i = at ? i + 1 : 0;
        k += !at;
    }
    return (found);
}

/*
** count_row
**
** Counts a row that holds a string in the share, and never answers that it
** found one: every row is to be read.
*/
static bool count_row(void *ctx, const uint8_t *row)
{
    module_pointing_t *p = ctx;

    p->rows += points_in(p->m, p->c, row);
    return (false);
}

/*
** module_count_pointing
**
** Answers how many rows of a type hold a string in a module's share of the
** persistent arena.
*/
uint32_t module_count_pointing(const bridge_t *b, const module_t *m,
    const declared_type_t *c)
{
    module_pointing_t p = { m, c, 0 };

    if (c->kind == BRIDGE_STRUCT)
        (void)storage_has_any(b->storage, c->type, count_row, &p);
    return (p.rows);
}
