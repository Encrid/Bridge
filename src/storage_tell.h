/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_tell.h -- an entity gone, and what each observer is owed for
** an entity, from what it carried and what it carries.
**
** data.md, *Observers*
*/

#ifndef BRIDGE_SRC_STORAGE_TELL_H
    #define BRIDGE_SRC_STORAGE_TELL_H

    #include "storage_move.h"
    #include "storage_value.h"

static inline bool is_alive(const storage_t *r, uint64_t id)
{
    uint32_t index = (uint32_t)id;

    if (!index || index >= r->high || r->places[index].table == DEAD)
        return (false);
    return (r->places[index].generation == (uint32_t)(id >> 32));
}

/*
** leave_apart
**
** Takes the entity at `index` out of every sparse set it is in, and takes
** from it every direct-indexed component it was given.
*/
static inline void leave_apart(storage_t *r, uint32_t index)
{
    uint32_t slot = 0;

    while (slot < r->set_room) {
        if (r->set_type[slot] && get_where(r, slot)[index])
            leave_set(r, slot, index);
        slot++;
    }
    slot = 0;
    while (slot < r->direct_room) {
        if (r->direct_type[slot] && set_has(get_given(r, slot), index))
            give_cell(r, slot, index, false);
        slot++;
    }
}

/*
** kill_entity
**
** Makes the entity at `index` gone: its row, and its index, which comes back
** under another generation.
*/
[[gnu::always_inline]] static inline void kill_entity(storage_t *r,
    uint32_t index)
{
    place_t *p = &r->places[index];
    uint32_t next = p->generation + 1;

    if (r->apart)
        leave_apart(r, index);
    if (p->table)
        remove_row(r, p->table, p->row);
    r->b->index[index] = (bridge_entity_t){ 0, 0, DEAD };
    p->table = DEAD;
    p->generation = next >= DEAD ? 1 : next;
    p->row = r->free_head;
    r->free_head = index;
}

/*
** get_owed
**
** Answers what one observer is owed for an entity: that it entered what the
** observer's terms match, left it, or matches and was attached one of them.
** An entity that carries nothing is in no table, and matches nothing.
*/
static inline uint32_t get_owed(const storage_t *r, const storage_terms_t *o,
    const seen_t *seen)
{
    const uint64_t *before = seen->before;
    const uint64_t *after = seen->after;
    bool was = before && !set_is_empty(r, before) && terms_match(o, before);
    bool is = after && !set_is_empty(r, after) && terms_match(o, after);
    uint32_t owed = was ? 0 : BRIDGE_ON_ENTER;
    uint32_t k = 0;

    if (!is)
        return (was ? BRIDGE_ON_LEAVE & o->events : 0);
    while (seen->attached && k < o->count) {
        if (o->oper[k] != BRIDGE_NOT && set_has(seen->attached, o->type[k]))
            owed |= BRIDGE_ON_ATTACH;
        k++;
    }
    return (owed & o->events);
}

/*
** tell_observers
**
** Tells each observer what it is owed for an entity, in the order the
** observers came.
*/
static inline void tell_observers(const storage_t *r, const seen_t *seen)
{
    const storage_terms_t *o;
    uint32_t owed;
    uint32_t i = 0;

    while (i < r->observer_count) {
        o = r->observers[i];
        owed = get_owed(r, o, seen);
        if (owed)
            o->heard(o->ctx, seen->id, (enum bridge_event)owed);
        i++;
    }
}

#endif
