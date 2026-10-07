/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/bridge_load.c -- a module loaded.
**
** lifecycle.md, *The runtime lifecycle*
*/

#include <stdlib.h>

#include "module.h"

static void keep_share(module_t *m, uint32_t at, uint32_t share)
{
    m->owned[0] = (range_t){ at, share };
    m->own_count = 1;
    m->own_next = 1;
    m->arena_share = share;
    m->arena_at = at;
    m->arena_left = share;
}

/*
** take_share
**
** Takes a module's share of the persistent arena, for as long as it is
** loaded: nobody else allocates from it. 1 with a record, and the module
** discarded, if there was no memory to keep track of it.
*/
static int take_share(bridge_t *b, module_t *m)
{
    uint32_t most = region_get_most(&b->arena_space);
    uint32_t share = ((uint32_t)m->persistent + 7u) & ~7u;
    uint32_t at;

    m->owned = calloc(1, sizeof(*m->owned));
    if (!m->owned || !region_make_room(&b->arena_space, b->arena_ranges + 1)) {
        (void)bridge_lacks(b, nullptr, "a module's share of the persistent "
            "arena");
        module_discard(b, m);
        return (1);
    }
    share = share > most ? most : share;
    if (!region_take(&b->arena_space, share, &at))
        unreachable();
    b->arena_ranges++;
    keep_share(m, b->arena + at, share);
    return (0);
}

/*
** fail_load
**
** Says that a load failed, and leaves storage as it was: the ids set aside
** for the module, what it took of the arena, and every component nobody
** else declared. Its systems go first, since a query must not outlive a
** component it names; what it queued goes with its queue. A schedule with
** its systems taken back out is one that was built before, so the rebuild
** cannot be refused, and asks for no memory.
**
** failure.md, *When something fails*
*/
static int fail_load(bridge_t *b, module_t *m, backend_err_t *err)
{
    say_error(b, "module '%s' did not load: %s", m->identity, err->text);
    if (m->on_error == BRIDGE_FAIL)
        bridge_stop(b, m->identity, "failed");
    module_drop_systems(b, m);
    storage_release(b->storage, m->ids, m->id_count);
    m->id_count = 0;
    m->id_next = 0;
    module_give_share(b, m);
    m->own_count = 0;
    module_release_types(b, m, false);
    b->layout_moved = 1;
    module_discard(b, m);
    (void)schedule_build(b, err->text, sizeof(err->text));
    return (1);
}

/*
** start_loaded
**
** Runs a module being loaded for the first time. init may spawn, so its ids
** are set aside before it runs; and a new system or observer that conflicts
** unordered is refused.
**
** budgets.md, *Budgets*
*/
static int start_loaded(const module_check_t *k, module_t *m,
    backend_err_t *err)
{
    bridge_t *b = k->b;
    module_start_t st = { k->a, k->demands, "init" };

    if (block_refill(b, m))
        return (module_fail(err, "its spawns of %lld do not fit under "
            "[budget] entities of %u", (long long)k->cfg->spawns,
            b->entities));
    if (module_start(m, &st, err))
        return (1);
    return (schedule_build(b, err->text, sizeof(err->text)));
}

/*
** bridge_load
**
** Loads a module. Once it has a record a failure is the module's, and its
** on_error decides what it costs. It goes at the end of the modules: the
** order they loaded in is the order their changes are applied in. What init
** queued is applied, and its spans are published. A module that is in and
** that nothing will ever run is not a load that succeeded.
*/
int bridge_load(bridge_t *b, const bridge_artifact_t *a)
{
    module_check_t k = { b, a };
    backend_err_t err = {};
    module_t **last = &b->modules;
    module_t *m;

    if (bridge_check_call(b, a->cfg.identity) || module_check(&k))
        return (1);
    m = module_check_room(&k) ? nullptr : module_record(b, a, k.on);
    if (!m || (m->persistent > 0 && take_share(b, m)))
        return (1);
    if (start_loaded(&k, m, &err))
        return (fail_load(b, m, &err));
    while (*last)
        last = &(*last)->next;
    *last = m;
    b->module_count++;
    b->layout_moved = 1;
    bridge_resume(b, m);
    block_barrier(b);
    return (b->stopped_by ? 1 : 0);
}
