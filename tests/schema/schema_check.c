/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/schema/schema_check.c -- one descriptor read, and held against
** what was wanted of it.
*/

#include <stdio.h>
#include <string.h>

#include "schema_test.h"

/*
** schema_find_named
**
** What the reader calls for a type a field names: a namespace that has
** declared three types already.
*/
bool schema_find_named([[maybe_unused]] void *ctx, const char *name,
    schema_named_t *named)
{
    if (!strcmp(name, "Vec2"))
        *named = (schema_named_t){ BRIDGE_STRUCT, 8, 4, nullptr };
    if (!strcmp(name, "Team"))
        *named = (schema_named_t){ BRIDGE_ENUM, 4, 4, nullptr };
    if (!strcmp(name, "Flags"))
        *named = (schema_named_t){ BRIDGE_BITMASK, 4, 4, nullptr };
    return (!strcmp(name, "Vec2") || !strcmp(name, "Team")
        || !strcmp(name, "Flags"));
}

/*
** schema_say_failed
**
** Says that a descriptor did not read as wanted. Answers 1.
*/
int schema_say_failed(const char *text, const char *what)
{
    printf("FAIL: %s: %s\n", text, what);
    return (1);
}

static bool is_wanted(const schema_field_t *f, const want_field_t *want)
{
    return (!strcmp(f->name, want->name) && f->offset == want->offset
        && f->count == want->count && f->kind == want->kind);
}

/*
** schema_check_struct
**
** Reads a struct's descriptor and holds its size, its alignment and each
** field against what this compiler made of the same struct.
*/
int schema_check_struct(const want_struct_t *w)
{
    char why[160];
    schema_source_t source = { BRIDGE_STRUCT, w->text, schema_find_named,
        nullptr, why, sizeof(why) };
    schema_t s;
    int failed = 0;
    uint32_t i = 0;

    if (schema_parse(&source, &s))
        return (schema_say_failed(w->text, why));
    if (s.size != w->size || s.align != w->align || s.field_count != w->count)
        failed = schema_say_failed(w->text, "its size, alignment or field "
            "count");
    while (i < w->count && i < s.field_count) {
        if (!is_wanted(&s.fields[i], &w->want[i]))
            failed += schema_say_failed(w->text, w->want[i].name);
        i++;
    }
    schema_free(&s);
    return (failed);
}

/*
** schema_check_constants
**
** Reads an enum's or a bitmask's descriptor and holds its width and each
** constant against what is wanted.
*/
int schema_check_constants(const want_constants_t *w)
{
    char why[160];
    schema_source_t source = { w->kind, w->text, schema_find_named, nullptr,
        why, sizeof(why) };
    schema_t s;
    int failed = 0;
    uint32_t i = 0;

    if (schema_parse(&source, &s))
        return (schema_say_failed(w->text, why));
    if (s.size != w->size || s.align != w->size
        || s.constant_count != w->count)
        failed = schema_say_failed(w->text, "its size or how many constants");
    while (i < w->count && i < s.constant_count) {
        if (s.constants[i].value != w->want[i])
            failed += schema_say_failed(w->text, s.constants[i].name);
        i++;
    }
    schema_free(&s);
    return (failed);
}
