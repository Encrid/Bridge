/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_refold.c -- the first half of a move: what each entity that
** carries a moved type will carry, with a table and room found for it.
*/

#include "storage_convert.h"
#include "storage_forget.h"
#include "storage_room.h"

/*
** fold_in
**
** Counts the entity at `index` among those this move changes, folded from
** what it carries.
*/
static void fold_in(storage_t *r, uint32_t index)
{
    if (r->folded[index] & TOUCHED)
        return;
    r->folded[index] = TOUCHED;
    r->changed[r->changed_count] = index;
    r->changed_count++;
    get_carried(r, index, get_work(r, index));
}

/*
** retype
**
** Folds one entity that carries a moved type: it carries the old type no
** longer, and the new one instead. A singleton is on no entity: what the one
** that carried it had becomes index 0's.
*/
static void retype(storage_t *r, uint32_t index, void *ctx)
{
    const storage_move_t *move = ctx;
    uint32_t to = move->to->kept == BRIDGE_SINGLETON ? 0 : index;

    fold_in(r, index);
    set_take(get_work(r, index), move->from->id);
    fold_in(r, to);
    set_put(get_work(r, to), move->to->id);
}

/*
** is_short
**
** Answers whether any changed entity was left as it was, for want of a
** table or of columns.
*/
static bool is_short(const storage_t *r)
{
    uint32_t i = 0;

    while (i < r->changed_count && !(r->folded[r->changed[i]] & DROPPED))
        i++;
    return (i < r->changed_count);
}

/*
** fold_singles
**
** Folds index 0 first for each move to a singleton that has something to
** move, so that a singleton has its place before the value that goes to it.
*/
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

/*
** storage_fold_moves
**
** Folds what each entity that carries one of the moved types will carry,
** and finds a table and room for each, with nothing moved.
*/
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
