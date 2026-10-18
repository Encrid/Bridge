/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "reload.h"

static void retire_old(reloading_t *r)
{
    bridge_t *b = r->b;
    module_t *old = r->old;
    uint32_t i = 0;

    while (i < b->system_count) {
        if (b->systems[i]->m == old)
            abi_park(b, b->systems[i]);
        i++;
    }
    r->side_by_side = (r->on->capabilities & BACKEND_SIDE_BY_SIDE) != 0;
    if (r->side_by_side)
        return;
    module_shut_down(b, old);
    if (old->instance)
        r->on->api.instance_destroy(old->instance);
    if (old->image)
        r->on->api.module_destroy(old->image);
    old->instance = nullptr;
    old->image = nullptr;
}

static void forget_config(module_t *n)
{
    uint32_t i = 0;

    while (i < n->declared_count && n->declared[i] != n->config)
        i++;
    if (i == n->declared_count)
        n->config = nullptr;
}

static void swap(reloading_t *r)
{
    bridge_t *b = r->b;
    module_t *old = r->old;
    module_t *n = r->n;
    module_t **link = &b->modules;

    old->lender = n;
    if (r->side_by_side)
        module_shut_down(b, old);
    module_drop_systems(b, old);
    module_release_types(b, old, true);
    old->id_count = 0;
    old->id_next = 0;
    forget_config(n);
    while (*link != old)
        link = &(*link)->next;
    n->next = old->next;
    *link = n;
    module_discard(b, old);
    (void)schedule_build(b, r->err.text, sizeof(r->err.text));
    b->layout_moved = 1;
}

static int start_new(reloading_t *r)
{
    bridge_t *b = r->b;
    module_start_t st = { r->a, r->demands, "reload" };
    backend_err_t *err = &r->err;

    r->n->reloads = r->old;
    if (module_start(r->n, &st, err))
        return (1);
    if (schedule_build(b, err->text, sizeof(err->text)))
        return (1);
    return (reload_move_rows(r));
}

int bridge_reload_now(bridge_t *b, const bridge_artifact_t *a)
{
    reloading_t r = { .b = b, .a = a };

    if (reload_check(&r) || reload_grow_share(&r) || reload_make_record(&r))
        return (1);
    retire_old(&r);
    reload_take_cursors(&r);
    if (start_new(&r))
        return (reload_fail(&r));
    swap(&r);
    bridge_resume(b, r.n);
    block_barrier(b);
    return (b->stopped_by ? 1 : 0);
}
