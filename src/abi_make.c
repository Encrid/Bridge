/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/abi_make.c -- a declared type made: everything it needs is had before
** any of it is kept.
**
** failure.md, *Out of memory*
*/

#include <stdlib.h>
#include <string.h>

#include "abi_declare.h"

static void free_type(declared_type_t *c)
{
    if (!c)
        return;
    free(c->name);
    free(c->zero);
    free(c->deps);
    free(c);
}

/*
** alloc_type
**
** Allocates a declared type with its name, its zero value, room for the
** types its fields name, which are no more than its fields, and its place
** among the bridge's types. nullptr, with nothing kept, if there was not the
** memory for all of it.
*/
static declared_type_t *alloc_type(declaring_t *g)
{
    bridge_t *b = g->m->b;
    declared_type_t *c = calloc(1, sizeof(*c));
    void *types = realloc(b->types, (b->type_count + 1) * sizeof(*b->types));
    uint32_t held = g->s.field_count;

    if (types)
        b->types = types;
    if (c) {
        c->name = copy_text(g->name, strlen(g->name));
        c->zero = calloc(1, g->d.size ? g->d.size : 1);
        c->deps = calloc(held ? held : 1, sizeof(*c->deps));
    }
    if (c && c->name && c->zero && c->deps && types)
        return (c);
    free_type(c);
    return (nullptr);
}

/*
** say_unmade
**
** Frees what was had of a type that is not made, and refuses it: there was
** no memory for it, or storage has no room for a singleton's value or a
** direct-indexed type's cells.
*/
static int say_unmade(declaring_t *g, declared_type_t *c)
{
    const bridge_t *b = g->m->b;

    schema_free(&g->s);
    if (!c)
        return (abi_refuse(g->m, "out of memory: there was none for type "
            "'%s'", g->name));
    free_type(c);
    if (g->d.storage == BRIDGE_SINGLETON)
        return (abi_refuse(g->m, "type '%s' is a singleton, and [budget] "
            "columns of %u bytes has no room for its %u bytes", g->name,
            b->columns_len, g->d.size));
    return (abi_refuse(g->m, "type '%s' is direct-indexed, and [budget] "
        "columns of %u bytes has no room for a cell of %u bytes for each of "
        "%u entities", g->name, b->columns_len, g->d.size, b->entities));
}

/*
** keep_type
**
** Keeps a made type among the bridge's, with what the declaration stated,
** and holds each declared type a field of it names for as long as it is.
*/
static void keep_type(declaring_t *g, declared_type_t *c)
{
    bridge_t *b = g->m->b;
    declared_type_t *held;
    uint32_t k = 0;

    c->s = g->s;
    c->kind = g->d.kind;
    c->storage = g->d.storage;
    c->size = g->d.size;
    c->align = g->d.align;
    c->fingerprint = g->print;
    c->replaces = g->was;
    b->types[b->type_count++] = c;
    while (k < c->s.field_count) {
        held = c->s.fields[k].type;
        if (held) {
            c->deps[c->dep_count++] = held;
            held->users++;
        }
        k++;
    }
}

/*
** abi_make_type
**
** Makes the type a declaration states.
*/
int abi_make_type(declaring_t *g)
{
    const bridge_declaration_t *d = &g->d;
    storage_t *storage = g->m->b->storage;
    declared_type_t *c;

    if (abi_read_layout(g))
        return (1);
    c = alloc_type(g);
    if (c)
        c->type = storage_declare(storage, d->size, d->align, d->storage);
    if (c && c->type) {
        if (g->was)
            g->was->superseded = true;
        keep_type(g, c);
        g->c = c;
        return (0);
    }
    return (say_unmade(g, c));
}
