/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/abi_layout.c -- the descriptor of a type being declared, read and
** laid out, and held against what its module's compiler made of it.
**
** schema.md, *There is no schema format*
*/

#include <stdlib.h>
#include <string.h>

#include "abi_declare.h"

/*
** parse_descriptor
**
** Reads the descriptor of the type being declared. A field's type is a bare
** C name, looked for in the declarer's namespace: `Vec3` in game:Body is
** game:Vec3. The name is cut at its namespace while the text is read.
**
** schema.md, *The type set*
** failure.md, *Out of memory*
*/
static int parse_descriptor(declaring_t *g)
{
    char *desc = copy_text(g->desc, g->d.desc_len);
    char why[200];
    schema_source_t source = { g->d.kind, desc, abi_find_type, g, why };
    int refused;

    if (!desc)
        return (abi_refuse(g->m, "out of memory: there was none for the "
            "descriptor of '%s'", g->name));
    source.why_len = sizeof(why);
    *g->bare = 0;
    refused = schema_parse(&source, &g->s);
    *g->bare = ':';
    free(desc);
    if (!refused)
        return (0);
    return (abi_refuse(g->m, "type '%s': its descriptor %s", g->name, why));
}

/*
** check_size
**
** Refuses a type that its descriptor lays out as other bytes, or aligned
** otherwise, than its module's compiler did.
**
** schema.md, *There is no schema format*
*/
static int check_size(declaring_t *g)
{
    const bridge_declaration_t *d = &g->d;

    if (g->s.size == d->size && g->s.align == d->align)
        return (0);
    return (abi_refuse(g->m, "type '%s' is %u bytes aligned to %u in the "
        "module and %u aligned to %u by its descriptor", g->name, d->size,
        d->align, g->s.size, g->s.align));
}

/*
** check_room
**
** Carries a replaced type's values over to this one's layout, where it
** changed, and refuses a type that is one more than a budget allows.
**
** budgets.md, *Budgets*
*/
static int check_room(declaring_t *g)
{
    module_t *m = g->m;
    const bridge_t *b = m->b;
    declared_type_t *was = g->was;
    enum bridge_storage kept = g->d.storage;
    bool changed = was && was->fingerprint != g->print;

    changed = changed || (was && abi_names_superseded(was));
    if (changed && abi_relay(m, was, g->d.kind, &g->s))
        return (1);
    if (b->type_count >= b->max_types)
        return (abi_refuse(m, "type '%s' is one more than [budget] types of "
            "%u", g->name, b->max_types));
    if (kept == BRIDGE_SPARSE_SET && !storage_count_free_sets(b->storage))
        return (abi_refuse(m, "type '%s' is one more than [budget] "
            "sparse_sets of %u", g->name, b->sparse_sets));
    if (kept != BRIDGE_DIRECT_INDEXED || storage_count_free_direct(b->storage))
        return (0);
    return (abi_refuse(m, "type '%s' is one more than [budget] "
        "direct_indexed of %u", g->name, b->direct_indexed));
}

/*
** abi_read_layout
**
** Reads and lays out the descriptor of the type being declared, and checks
** it against what the module stated and against the budgets. A tag has
** nothing to read: it is what it was stated to be, which is nothing.
*/
int abi_read_layout(declaring_t *g)
{
    memset(&g->s, 0, sizeof(g->s));
    g->s.align = g->d.align;
    if (g->d.kind != BRIDGE_TAG && parse_descriptor(g))
        return (1);
    if (!check_size(g) && !check_room(g))
        return (0);
    schema_free(&g->s);
    return (1);
}
