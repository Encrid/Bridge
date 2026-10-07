/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/storage_apply.c -- a barrier, worked an entity at a time: the queues
** folded, room found, observers told, each entity placed once, the values
** written.
**
** storage.md, *A barrier is worked an entity at a time*
*/

#include "storage_change.h"
#include "storage_forget.h"
#include "storage_place.h"
#include "storage_room.h"
#include "storage_runs.h"
#include "storage_settle.h"

/*
** fold_all
**
** The first step: what each changed entity will carry, and nothing moved.
*/
static void fold_all(storage_t *r)
{
    module_t *m;
    uint32_t at;
    uint32_t end;

    reset_marks(r->b);
    m = take_next_run(r->b, &at, &end);
    while (m) {
        fold_run(r, m, at, end);
        m = take_next_run(r->b, &at, &end);
    }
}

/*
** tell_all
**
** The third step: what observers are owed, for each entity that was not
** left as it was, from what it carried against what it will.
*/
static void tell_all(storage_t *r)
{
    seen_t seen = { .before = r->before };
    uint32_t index;
    uint32_t i = 0;

    while (r->observer_count && i < r->changed_count) {
        index = r->changed[i];
        seen.id = (uint64_t)r->places[index].generation << 32 | index;
        seen.after = get_work(r, index);
        if (r->folded[index] & DESTROYED)
            seen.after = nullptr;
        seen.attached = get_attached(r, index);
        if (!(r->folded[index] & DROPPED)) {
            get_carried(r, index, r->before);
            tell_observers(r, &seen);
        }
        i++;
    }
}

/*
** place_all
**
** The fourth step: each entity placed, once.
*/
static void place_all(storage_t *r)
{
    uint32_t i = 0;

    while (i < r->changed_count) {
        if (!(r->folded[r->changed[i]] & DROPPED))
            place_one(r, r->changed[i]);
        i++;
    }
}

/*
** write_all
**
** The fifth step: the values, in the order they were asked, so that the
** last one written is the one that stays.
*/
static void write_all(storage_t *r)
{
    module_t *m;
    change_t ch;
    uint32_t at;
    uint32_t end;

    reset_marks(r->b);
    m = take_next_run(r->b, &at, &end);
    while (m) {
        while (at < end) {
            memcpy(&ch, m->queue + at, sizeof(ch));
            write_value(r, m, &ch, m->queue + at + sizeof(ch));
            at += (uint32_t)sizeof(ch) + ((ch.size + 7u) & ~7u);
        }
        m = take_next_run(r->b, &at, &end);
    }
}

/*
** storage_apply
**
** Applies every module's queue, in the order the calls that filled them ran.
** An entity that cannot be placed is left as it was, and its module told.
*/
void storage_apply(storage_t *r)
{
    fold_all(r);
    reserve_all(r);
    tell_all(r);
    place_all(r);
    write_all(r);
    settle_all(r);
}
