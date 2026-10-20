/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "abi_declare.h"

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

int abi_check_name(module_t *m, const char *what, const char *name,
    uint32_t len)
{
    if (!len || len > NAME_MOST)
        return (abi_refuse(m, "%s: its name has %u bytes, and a name has 1 to "
            "%u", what, len, NAME_MOST));
    if (name && memchr(name, 0, len))
        return (abi_refuse(m, "%s: its name has a zero byte", what));
    return (0);
}

int abi_check_stated(declaring_t *g)
{
    const char *desc = g->d.kind == BRIDGE_TAG ? "" : g->desc;

    if (check_kind(g) || check_policy(g))
        return (1);
    g->print = abi_hash_text(desc, g->d.desc_len) ^ g->d.kind;
    return (0);
}
