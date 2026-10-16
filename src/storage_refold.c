/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_convert.h"
#include "storage_forget.h"
#include "storage_room.h"

static void fold_in(storage_t *r, uint32_t index)
{
    if (r->folded[index] & TOUCHED)
        return;
    r->folded[index] = TOUCHED;
    r->changed[r->changed_count] = index;
    r->changed_count++;
    get_carried(r, index, get_work(r, index));
}

static void retype(storage_t *r, uint32_t index, void *ctx)
{
    const storage_move_t *move = ctx;
    uint32_t to = move->to->kept == BRIDGE_SINGLETON ? 0 : index;

    fold_in(r, index);
    set_take(get_work(r, index), move->from->id);
    fold_in(r, to);
    set_put(get_work(r, to), move->to->id);
}

static bool is_short(const storage_t *r)
{
    uint32_t i = 0;

    while (i < r->changed_count && !(r->folded[r->changed[i]] & DROPPED))
        i++;
    return (i < r->changed_count);
}

static void fold_singles(const convert_t *c)
{
    const storage_move_t *move;
    uint32_t k = 0;

    while (k < c->count) {
        move = &c->moves[k];
        if (move->to->kept == BRIDGE_SINGLETON
            && storage_count_carriers(c->r, move->from))
            fold_in(c->r, 0);
        k++;
    }
}

bool storage_fold_moves(const convert_t *c)
{
    storage_t *r = c->r;
    uint32_t k = 0;

    fold_singles(c);
    while (k < c->count) {
        storage_visit_carriers(r, c->moves[k].from, retype,
            (void *)(uintptr_t)&c->moves[k]);
        k++;
    }
    reserve_all(r);
    if (!is_short(r))
        return (true);
    forget_all(r);
    return (false);
}
