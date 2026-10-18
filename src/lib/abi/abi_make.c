/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "abi_declare.h"

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
    module_free_type(c);
    return (nullptr);
}

static int say_unmade(declaring_t *g, declared_type_t *c)
{
    const bridge_t *b = g->m->b;

    schema_free(&g->s);
    free(g->text);
    g->text = nullptr;
    if (!c)
        return (abi_refuse(g->m, "out of memory: there was none for type "
            "'%s'", g->name));
    module_free_type(c);
    if (g->d.storage == BRIDGE_SINGLETON)
        return (abi_refuse(g->m, "type '%s' is a singleton, and [budget] "
            "columns of %u bytes has no room for its %u bytes", g->name,
            b->columns_len, g->d.size));
    return (abi_refuse(g->m, "type '%s' is direct-indexed, and [budget] "
        "columns of %u bytes has no room for a cell of %u bytes for each of "
        "%u entities", g->name, b->columns_len, g->d.size, b->entities));
}

static void hold_named(declared_type_t *c)
{
    declared_type_t *held;
    uint32_t k = 0;

    while (k < c->s.field_count) {
        held = c->s.fields[k].type;
        if (held) {
            c->deps[c->dep_count++] = held;
            held->users++;
        }
        k++;
    }
}

static void keep_type(declaring_t *g, declared_type_t *c)
{
    bridge_t *b = g->m->b;

    c->s = g->s;
    c->kind = g->d.kind;
    c->storage = g->d.storage;
    c->size = g->d.size;
    c->align = g->d.align;
    c->fingerprint = g->print;
    c->desc = g->text;
    c->desc_len = g->d.desc_len;
    g->text = nullptr;
    c->replaces = g->was;
    b->types[b->type_count++] = c;
    hold_named(c);
}

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
