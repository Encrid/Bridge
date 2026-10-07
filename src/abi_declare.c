/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/abi_declare.c -- the entry a module declares a type by.
**
** abi.md, *The ABI*
*/

#include <stdlib.h>
#include <string.h>

#include "abi_declare.h"
#include "abi_entry.h"

/*
** read_stated
**
** Copies what a declaration states out of the scratch arena, since a module
** may have put it at any offset, and its name with it.
*/
static int read_stated(declaring_t *g, uint64_t at)
{
    module_t *m = g->m;
    bridge_declaration_t *d = &g->d;
    const void *stated = get_scratch(m, at, sizeof(*d));
    const char *name;

    if (!stated)
        return (abi_refuse(m, "declare: what it states lies outside the "
            "scratch arena"));
    memcpy(d, stated, sizeof(*d));
    if (!m->in_init)
        return (abi_refuse(m, "declare is called outside init"));
    name = get_scratch(m, d->name, d->name_len);
    g->desc = get_scratch(m, d->desc, d->desc_len);
    if (!name || (!g->desc && d->kind != BRIDGE_TAG) || !d->name_len
        || d->name_len >= sizeof(g->name))
        return (abi_refuse(m, "declare: a name or a descriptor lies outside "
            "the scratch arena"));
    memcpy(g->name, name, d->name_len);
    g->name[d->name_len] = 0;
    return (0);
}

/*
** make_slots
**
** Makes room for one more of what a module declares, and the set of what it
** may write, before anything is made that would have to be unmade.
**
** failure.md, *Out of memory*
*/
static int make_slots(module_t *m)
{
    size_t n = m->declared_count + 1;
    void *declared = realloc(m->declared, n * sizeof(*m->declared));
    void *policy = realloc(m->policy, n * sizeof(*m->policy));
    void *writes = realloc(m->may_write, n * sizeof(*m->may_write));

    if (declared)
        m->declared = declared;
    if (policy)
        m->policy = policy;
    if (writes)
        m->may_write = writes;
    if (declared && policy && writes && storage_make_writable(m->b->storage, m))
        return (0);
    return (abi_refuse(m, "out of memory: there was none for what it "
        "declares"));
}

/*
** abi_is_writable
**
** Answers whether a module may write a type: one of its own namespace, or
** one a grant lets it write.
*/
bool abi_is_writable(const module_t *m, const declared_type_t *c)
{
    return (!abi_is_foreign(m, c->name)
        || abi_is_listed(m->write, m->write_count, c->name));
}

/*
** take_slot
**
** Answers the slot a module has a type in, which is a new one unless it
** declared the type before: declare is idempotent.
*/
static uint32_t take_slot(declaring_t *g)
{
    module_t *m = g->m;
    declared_type_t *c = g->c;
    uint32_t i = 0;

    while (i < m->declared_count && m->declared[i] != c)
        i++;
    if (i < m->declared_count)
        return (i);
    m->policy[i] = g->d.on_unload;
    m->declared[i] = c;
    m->may_write[i] = abi_is_writable(m, c);
    if (m->may_write[i])
        storage_let_write(m->b->storage, m, c->type);
    c->declarers++;
    m->b->layout_moved = 1;
    m->declared_count++;
    return (i);
}

/*
** abi_declare
**
** declare(at) -> slot.
*/
int abi_declare(void *ud, const uint64_t *at, uint64_t *r)
{
    declaring_t g = { .m = ud };

    if (read_stated(&g, at[0]) || make_slots(g.m) || abi_check_stated(&g))
        return (1);
    if (abi_replace_type(&g))
        return (1);
    if (!g.c && abi_make_type(&g))
        return (1);
    r[0] = take_slot(&g);
    return (0);
}
