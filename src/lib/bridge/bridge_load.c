/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

static int take_share(bridge_world_t *w, module_t *m)
{
    uint32_t share = (uint32_t)module_round_share(m->persistent);
    uint32_t at;

    m->owned = calloc(1, sizeof(*m->owned));
    if (!m->owned || !region_make_room(&w->arena_space, w->arena_ranges + 1)) {
        (void)bridge_lacks(w, nullptr, "a module's share of the persistent "
            "arena");
        module_discard(w, m);
        return (1);
    }
    (void)region_take(&w->arena_space, share, &at);
    w->arena_ranges++;
    keep_share(m, w->arena + at, share);
    return (0);
}

static int fail_load(bridge_world_t *w, module_t *m, backend_err_t *err)
{
    if (m->priv)
        block_drain(w, m);
    say_error(w, "module '%s' did not load: %s", m->identity, err->text);
    if (m->on_error == BRIDGE_FAIL)
        bridge_stop(w, m->identity, "failed");
    module_drop_systems(w, m);
    storage_release(w->storage, m->ids, m->id_count);
    m->id_count = 0;
    m->id_next = 0;
    module_give_share(w, m->owned, m->own_count);
    m->own_count = 0;
    module_release_types(w, m, false);
    w->layout_moved = 1;
    module_discard(w, m);
    (void)schedule_build(w, err->text, sizeof(err->text));
    return (1);
}

static int start_loaded(const module_check_t *k, module_t *m,
    backend_err_t *err)
{
    bridge_world_t *w = k->w;
    module_start_t st = { k->a, k->demands, "init" };

    if (block_refill(w, m))
        return (module_fail(err, "its spawns of %lld do not fit under "
            "[budget] entities of %u", (long long)k->cfg->spawns,
            w->entities));
    if (module_start(m, &st, err))
        return (1);
    if (schedule_build(w, err->text, sizeof(err->text)))
        return (1);
    return (config_seed(m, &k->a->cfg, err, true));
}

int bridge_load_now(bridge_world_t *w, const bridge_artifact_t *a)
{
    module_check_t k = { w, a };
    backend_err_t err = {};
    module_t **last = &w->modules;
    module_t *m;

    if (bridge_check_call(w, a->cfg.identity) || module_check(&k))
        return (1);
    m = module_check_room(&k) ? nullptr : module_record(w, a, k.on);
    if (!m || (m->persistent > 0 && take_share(w, m)))
        return (1);
    if (start_loaded(&k, m, &err))
        return (fail_load(w, m, &err));
    while (*last)
        last = &(*last)->next;
    *last = m;
    w->module_count++;
    w->layout_moved = 1;
    bridge_resume(w, m);
    block_barrier(w);
    return (w->stopped_by ? 1 : 0);
}
