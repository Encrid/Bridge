/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/storage_change.h -- a barrier's first step: each queued change folded
** into what its entity will carry, and nothing moved.
*/

#ifndef BRIDGE_SRC_STORAGE_CHANGE_H
    #define BRIDGE_SRC_STORAGE_CHANGE_H

    #include "storage_stake.h"
    #include "storage_tell.h"

/*
** is_foldable
**
** Answers whether a change is of something there is. A singleton is given
** and taken with the id 0, which is no entity: it is folded as the one thing
** index 0 carries. Any other change is of an entity that is alive and that
** this barrier has not destroyed already.
**
** storage.md, *A singleton*
*/
[[gnu::always_inline]] static inline bool is_foldable(const storage_t *r,
    const change_t *ch)
{
    uint32_t index = (uint32_t)ch->id;

    if (ch->op != CHANGE_DESTROY && ch->c->type->kept == BRIDGE_SINGLETON)
        return (ch->id == 0);
    return (is_alive(r, ch->id) && !(r->folded[index] & DESTROYED));
}

/*
** begin_entity
**
** Counts the entity at `index` among those this barrier changes, the first
** time a change of it is met.
*/
[[gnu::always_inline]] static inline void begin_entity(storage_t *r,
    uint32_t index)
{
    if (r->folded[index] & TOUCHED)
        return;
    r->folded[index] = TOUCHED;
    r->changed[r->changed_count] = index;
    r->changed_count++;
    start_fold(r, index);
    if (r->observer_count)
        set_clear(r, get_attached(r, index));
}

/*
** fold_change
**
** Folds one change into what its entity will carry.
*/
[[gnu::always_inline]] static inline void fold_change(storage_t *r,
    module_t *m, const change_t *ch)
{
    uint32_t index = (uint32_t)ch->id;
    uint32_t type = ch->op == CHANGE_DESTROY ? 0 : ch->c->type->id;
    bool attach = ch->op == CHANGE_ATTACH;

    if (!is_foldable(r, ch))
        return;
    begin_entity(r, index);
    if (ch->op == CHANGE_DESTROY) {
        fold_destroy(r, m, index);
        return;
    }
    set_write(get_work(r, index), type, attach);
    if (r->observer_count)
        set_write(get_attached(r, index), type, attach);
}

/*
** fold_run
**
** Folds one run of a module's queue: the changes of one of its turns, from
** `at` to `end`, in the order it asked them.
*/
[[gnu::always_inline]] static inline void fold_run(storage_t *r, module_t *m,
    uint32_t at, uint32_t end)
{
    change_t ch;

    while (at < end) {
        memcpy(&ch, m->queue + at, sizeof(ch));
        at += (uint32_t)sizeof(ch) + ((ch.size + 7u) & ~7u);
        fold_change(r, m, &ch);
    }
}

#endif
