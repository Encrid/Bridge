/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/abi_relay_fields.c -- the fields of a struct as it was, matched by
** name to those of what it becomes.
*/

#include <stdio.h>
#include <string.h>

#include "abi_declare.h"

static const schema_field_t *find_field(const schema_t *s, const char *name)
{
    const schema_field_t *found = nullptr;
    uint32_t k = 0;

    while (k < s->field_count) {
        if (!strcmp(s->fields[k].name, name))
            found = &s->fields[k];
        k++;
    }
    return (found);
}

/*
** is_same_type
**
** Answers whether a field is of the type it was: the same kind; and where
** it names a declared type, that one, or what this reload has made of it;
** and, but for a struct, of the same size.
*/
static bool is_same_type(const schema_field_t *old, const schema_field_t *new)
{
    const declared_type_t *held = old->type;
    const declared_type_t *holds = new->type;

    if (old->kind != new->kind)
        return (false);
    if (!held)
        return (true);
    if (holds != held && holds->replaces != held)
        return (false);
    return (holds->size == held->size || old->kind == SCHEMA_STRUCT);
}

/*
** relay_structs
**
** Matches a field that is a struct this reload changed, element by element:
** the same rules, one level down.
*/
static int relay_structs(relaying_t *l, const relay_at_t *at,
    const schema_field_t *old, const schema_field_t *new)
{
    const declared_type_t *held = old->type;
    const declared_type_t *holds = new->type;
    uint32_t count = old->count < new->count ? old->count : new->count;
    relay_at_t inner = { &held->s, &holds->s, 0, 0, at->path };
    uint32_t k = 0;

    while (k < count) {
        inner.from = at->from + old->offset + k * held->size;
        inner.to = at->to + new->offset + k * holds->size;
        if (abi_relay_fields(l, &inner))
            return (1);
        k++;
    }
    return (0);
}

/*
** relay_field
**
** Matches one field of a struct as it was. `at` names the field, by its
** path. One that is gone, or an array that got shorter, is a loss.
**
** lifetime.md, *When a component layout changed*
*/
static int relay_field(relaying_t *l, const relay_at_t *at,
    const schema_field_t *old)
{
    const schema_field_t *new = find_field(at->is, old->name);
    uint32_t count = new && new->count < old->count ? new->count : old->count;

    if (!new) {
        abi_say_lost(l, at->path);
        return (0);
    }
    if (!is_same_type(old, new))
        return (abi_refuse(l->m, "field '%s' is of another type than it was, "
            "and bytes of one type are not a value of another", at->path));
    if (new->count < old->count)
        abi_say_lost(l, at->path);
    if (old->kind == SCHEMA_STRUCT && new->type != old->type)
        return (relay_structs(l, at, old, new));
    abi_add_run(l, at->from + old->offset, at->to + new->offset,
        count * abi_get_element_size(new));
    return (0);
}

/*
** abi_relay_fields
**
** Matches the fields of a struct as it was to those of what it becomes.
*/
int abi_relay_fields(relaying_t *l, const relay_at_t *at)
{
    relay_at_t field = *at;
    char name[200];
    uint32_t i = 0;

    field.path = name;
    while (i < at->was->field_count) {
        snprintf(name, sizeof(name), "%s.%s", at->path,
            at->was->fields[i].name);
        if (relay_field(l, &field, &at->was->fields[i]))
            return (1);
        i++;
    }
    return (0);
}
