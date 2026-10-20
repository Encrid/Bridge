/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "abi_entry.h"

int abi_spawn(void *ud, [[maybe_unused]] const uint64_t *a, uint64_t *r)
{
    module_t *m = ud;
    module_t *from = m->lender ? m->lender : m;

    if (from->id_next != from->id_count) {
        r[0] = from->ids[from->id_next];
        from->id_next++;
        return (0);
    }
    if (from->spawns <= 0)
        return (abi_refuse(m, "spawn: it stated that it spawns nothing"));
    if (from->ids_short)
        return (abi_refuse(m, "spawn: [budget] entities of %u is reached",
            m->w->entities));
    return (abi_refuse(m, "spawn: its spawns of %lld between two barriers "
        "are used up", (long long)from->spawns));
}

int abi_destroy(void *ud, const uint64_t *a, [[maybe_unused]] uint64_t *r)
{
    change_t ch = { a[0], nullptr, 0, CHANGE_DESTROY };

    if (!a[0])
        return (abi_refuse(ud, "destroy: the id 0 is no entity"));
    return (queue_change(ud, &ch, nullptr));
}

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

int abi_detach(void *ud, const uint64_t *a, [[maybe_unused]] uint64_t *r)
{
    module_t *m = ud;
    declared_type_t *c = check_written(m, a[1], "detach");
    change_t ch = { a[0], c, 0, CHANGE_DETACH };

    if (!c || check_owner(m, "detach", a[0], c))
        return (1);
    return (queue_change(m, &ch, nullptr));
}
