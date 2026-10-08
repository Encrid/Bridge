/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/abi_calls.c -- the entries a module calls in a phase that change an
** entity: spawn, destroy, attach, detach.
*/

#include "abi_entry.h"

/*
** abi_spawn
**
** spawn() -> id. Hands out one of the ids set aside for the module, or says
** which of three things left it with none.
**
** budgets.md, *What running out does*
*/
int abi_spawn(void *ud, [[maybe_unused]] const uint64_t *a, uint64_t *r)
{
    module_t *m = ud;

    if (m->id_next != m->id_count) {
        r[0] = m->ids[m->id_next];
        m->id_next++;
        return (0);
    }
    if (m->spawns <= 0)
        return (abi_refuse(m, "spawn: it stated that it spawns nothing"));
    if (m->ids_short)
        return (abi_refuse(m, "spawn: [budget] entities of %u is reached",
            m->b->entities));
    return (abi_refuse(m, "spawn: its spawns of %lld between two barriers "
        "are used up", (long long)m->spawns));
}

/*
** abi_destroy
**
** destroy(id). An id that has already died is nobody's mistake: two modules
** may both want it gone, and which of them is told at the barrier, where it
** is looked at.
*/
int abi_destroy(void *ud, const uint64_t *a, [[maybe_unused]] uint64_t *r)
{
    change_t ch = { a[0], nullptr, 0, CHANGE_DESTROY };

    return (queue_change(ud, &ch, nullptr));
}

/*
** check_written
**
** Answers the component a slot names, if the module may attach or detach
** it: it declared the slot, a grant lets it write the component, and what
** is running stated that it changes it. nullptr, with a refusal, otherwise.
**
** abi.md, *The ABI*
** grants.md, *Whose component it is*
*/
static declared_type_t *check_written(module_t *m, uint64_t slot,
    const char *entry)
{
    const bridge_system_t *s = m->running;
    declared_type_t *c = slot < m->declared_count ? m->declared[slot] : nullptr;

    if (!c) {
        abi_refuse(m, "%s names slot %llu, which it never declared", entry,
            (unsigned long long)slot);
        return (nullptr);
    }
    if (!m->may_write[slot]) {
        abi_refuse(m, "it may not %s '%s': no grant says so", entry, c->name);
        return (nullptr);
    }
    if (is_stated(s, c))
        return (c);
    abi_refuse(m, "%s '%s' may not %s '%s': it is not among the changes it "
        "stated", s->observer ? "observer" : "system", s->name, entry, c->name);
    return (nullptr);
}

/*
** abi_attach
**
** attach(id, slot, ptr). The value travels with it, and is as long as the
** type was declared; BRIDGE_ZEROED as `ptr` asks for a zeroed one, and is
** the one `ptr` a tag takes, since a tag has no value.
**
** abi.md, *The ABI*
** storage.md, *A tag*
*/
int abi_attach(void *ud, const uint64_t *a, [[maybe_unused]] uint64_t *r)
{
    module_t *m = ud;
    declared_type_t *c = check_written(m, a[1], "attach");
    change_t ch = { a[0], c, c ? c->size : 0, CHANGE_ATTACH };
    const void *value = c ? c->zero : nullptr;

    if (!c)
        return (1);
    if (!c->size && a[2] != BRIDGE_ZEROED)
        return (abi_refuse(m, "attach: '%s' is a tag, and is handed a value",
            c->name));
    if (a[2] != BRIDGE_ZEROED)
        value = get_scratch(m, a[2], c->size);
    if (!value)
        return (abi_refuse(m, "attach: the value lies outside the scratch "
            "arena"));
    if (check_owner(m, "attach", a[0], c))
        return (1);
    return (queue_change(m, &ch, value));
}

/*
** abi_detach
**
** detach(id, slot).
*/
int abi_detach(void *ud, const uint64_t *a, [[maybe_unused]] uint64_t *r)
{
    module_t *m = ud;
    declared_type_t *c = check_written(m, a[1], "detach");
    change_t ch = { a[0], c, 0, CHANGE_DETACH };

    if (!c || check_owner(m, "detach", a[0], c))
        return (1);
    return (queue_change(m, &ch, nullptr));
}
