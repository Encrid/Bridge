/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/abi_stated.c -- what a declaration states, checked before anything is
** made of it: every value is stated, and none is taken for a default.
*/

#include <string.h>

#include "abi_declare.h"

/*
** abi_hash_text
**
** Answers the fingerprint of `n` bytes of text: FNV-1a.
*/
uint64_t abi_hash_text(const char *s, size_t n)
{
    uint64_t h = 1'469'598'103'934'665'603ull;
    size_t i = 0;

    while (i < n) {
        h = (h ^ (uint8_t)s[i]) * 1'099'511'628'211ull;
        i++;
    }
    return (h);
}

/*
** check_kind
**
** Refuses a name with no namespace, a kind left unset, and a tag that
** states bytes or a descriptor.
**
** abi.md, *The ABI*
** storage.md, *A tag*
*/
static int check_kind(declaring_t *g)
{
    const bridge_declaration_t *d = &g->d;

    g->bare = strchr(g->name, ':');
    if (!g->bare)
        return (abi_refuse(g->m, "type '%s' has no namespace", g->name));
    if (d->kind < BRIDGE_STRUCT || d->kind > BRIDGE_TAG)
        return (abi_refuse(g->m, "type '%s': its kind is unset", g->name));
    if (d->kind == BRIDGE_TAG && (d->size || d->desc_len))
        return (abi_refuse(g->m, "type '%s' is a tag, and a tag has no bytes "
            "and no descriptor", g->name));
    return (0);
}

/*
** check_policy
**
** Refuses another namespace's name that no grant names, and a policy or a
** storage layout left unset.
**
** grants.md, *Whose component it is*
** abi.md, *The ABI*
*/
static int check_policy(declaring_t *g)
{
    module_t *m = g->m;
    const bridge_declaration_t *d = &g->d;
    bool granted = abi_is_listed(m->read, m->read_count, g->name)
        || abi_is_listed(m->write, m->write_count, g->name);

    if (abi_is_foreign(m, g->name) && !granted)
        return (abi_refuse(m, "it may not declare '%s': that is another "
            "namespace's, and no grant names it", g->name));
    if (d->on_unload < BRIDGE_STRIP || d->on_unload > BRIDGE_DELETE)
        return (abi_refuse(m, "type '%s': on_unload is unset", g->name));
    if (d->storage < BRIDGE_TABLE || d->storage > BRIDGE_SINGLETON)
        return (abi_refuse(m, "type '%s': its storage layout is unset",
            g->name));
    return (0);
}

/*
** abi_check_stated
**
** Refuses a declaration that states what it may not, and works out the
** fingerprint of what it states: of its descriptor, and of its kind.
*/
int abi_check_stated(declaring_t *g)
{
    const char *desc = g->d.kind == BRIDGE_TAG ? "" : g->desc;

    if (check_kind(g) || check_policy(g))
        return (1);
    g->print = abi_hash_text(desc, g->d.desc_len) ^ g->d.kind;
    return (0);
}
