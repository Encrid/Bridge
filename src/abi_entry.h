/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/abi_entry.h -- what the entries a module calls in a phase share: a
** change put on its queue, and whether it may make it.
*/

#ifndef BRIDGE_SRC_ABI_ENTRY_H
    #define BRIDGE_SRC_ABI_ENTRY_H

    #include <string.h>

    #include "abi.h"

/*
** get_scratch
**
** Answers `len` bytes of a module's scratch arena at `at`, or nullptr if
** they do not lie in it: the one memory a module can write that
** Bridge can read and bounds-check.
*/
static inline const void *get_scratch(const module_t *m, uint64_t at,
    uint64_t len)
{
    if (at < m->scratch || len > m->scratch_len)
        return (nullptr);
    if (at - m->scratch > m->scratch_len - len)
        return (nullptr);
    return (m->priv + at);
}

/*
** mark_turn
**
** Marks where in a module's queue the changes of the turn in progress
** begin, the first time one is queued in it. 1 if there is no room for the
** mark.
*/
static inline int mark_turn(module_t *m)
{
    if (m->mark_count && m->marks[m->mark_count - 1].turn == m->turn)
        return (0);
    if (m->mark_count == m->mark_room)
        return (abi_refuse(m, "out of memory: there was none for a mark in "
            "its queue"));
    m->marks[m->mark_count].turn = m->turn;
    m->marks[m->mark_count].at = m->queue_len;
    m->mark_count++;
    return (0);
}

/*
** queue_change
**
** Puts a change on a module's queue, with its value after it: the value is
** copied now, since the scratch arena it is in is reset before the change
** is applied. The queue is as long as the module stated. 1 if refused.
**
** budgets.md, *Nothing is allocated in a phase*
*/
static inline int queue_change(module_t *m, const change_t *ch,
    const void *value)
{
    uint32_t need = (uint32_t)sizeof(*ch) + ((ch->size + 7u) & ~7u);
    uint8_t *at = m->queue + m->queue_len;

    if (!m->queue_room)
        return (abi_refuse(m, "it stated that it queues no change"));
    if (need > m->queue_room - m->queue_len)
        return (abi_refuse(m, "its changes of %u bytes are used up",
            m->queue_room));
    if (mark_turn(m))
        return (1);
    memcpy(at, ch, sizeof(*ch));
    if (value)
        memcpy(at + sizeof(*ch), value, ch->size);
    m->queue_len += need;
    return (0);
}

/*
** is_stated
**
** Answers whether what is running in a module stated that it changes a
** component. What a system changes is what it stated, so that two that
** change one component were ordered when they loaded. A lifecycle call runs
** alone, and states nothing.
**
** schedule.md, *Order within a phase is never implicit*
*/
static inline bool is_stated(const bridge_system_t *s,
    const declared_type_t *c)
{
    uint32_t i = 0;

    if (!s)
        return (true);
    while (i < s->change_count && s->changes[i] != c)
        i++;
    return (i < s->change_count);
}

/*
** check_owner
**
** Refuses a change of a singleton that names an entity, and of anything
** else that names none. A singleton is on no entity, so it is given and
** taken with the id 0, which is no entity; and nothing else is.
**
** storage.md, *A singleton*
*/
static inline int check_owner(module_t *m, const char *entry, uint64_t id,
    const declared_type_t *c)
{
    if (c->storage == BRIDGE_SINGLETON && id)
        return (abi_refuse(m, "%s: '%s' is a singleton, and its id is 0",
            entry, c->name));
    if (c->storage != BRIDGE_SINGLETON && !id)
        return (abi_refuse(m, "%s: the id 0 is no entity, and '%s' is not a "
            "singleton", entry, c->name));
    return (0);
}

#endif
