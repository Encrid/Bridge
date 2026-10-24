/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "abi_declare.h"

static declared_type_t *find_named(const bridge_world_t *w, const char *name)
{
    declared_type_t *c = nullptr;
    uint32_t i = 0;

    while (i < w->type_count) {
        if (!strcmp(w->types[i]->name, name) && !w->types[i]->superseded)
            c = w->types[i];
        i++;
    }
    return (c);
}

static bool is_reloaded(const module_t *m, const declared_type_t *c)
{
    const module_t *old = m->reloads;
    uint32_t i = 0;

    if (!old)
        return (false);
    while (i < old->declared_count && old->declared[i] != c)
        i++;
    return (i < old->declared_count);
}

static int replace_changed(declaring_t *g)
{
    module_t *m = g->m;
    const declared_type_t *c = g->c;
    const declared_type_t *holder;
    const module_t *by = nullptr;
    abi_others_t others;

    abi_say_others(m, c, &others);
    if (others.text[0] || !is_reloaded(m, c))
        return (abi_refuse(m, "type '%s' has another component layout here: "
            "fingerprint %016llx, and it was declared with %016llx%s", g->name,
            (unsigned long long)g->print, (unsigned long long)c->fingerprint,
            others.text));
    holder = abi_find_holder(m, c, &by);
    if (holder)
        return (abi_refuse(m, "type '%s' has another component layout here, "
            "and '%s', which module '%s' declares, has a field of it", g->name,
            holder->name, by->identity));
    return (c->storage != g->d.storage && abi_check_move(m, c, g->d.storage));
}

static int replace_moved(declaring_t *g)
{
    module_t *m = g->m;
    const declared_type_t *c = g->c;
    abi_others_t others;

    abi_say_others(m, c, &others);
    if (others.text[0] || !is_reloaded(m, c))
        return (abi_refuse(m, "type '%s' is stated as %s here, and it was "
            "declared as %s%s", g->name, ABI_STORAGE_NAMES[g->d.storage],
            ABI_STORAGE_NAMES[c->storage], others.text));
    return (abi_check_move(m, c, g->d.storage));
}

int abi_replace_type(declaring_t *g)
{
    declared_type_t *c = find_named(g->m->w, g->name);
    bool changed = c && !abi_is_same_layout(c, g);

    g->c = c;
    changed = changed || (c && abi_names_superseded(c));
    if (!c)
        return (0);
    if (!changed && c->storage == g->d.storage)
        return (abi_check_size(g, c->size, c->align));
    if (changed ? replace_changed(g) : replace_moved(g))
        return (1);
    g->was = c;
    g->c = nullptr;
    free(c->runs);
    c->runs = nullptr;
    c->run_count = 0;
    return (0);
}
