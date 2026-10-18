/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_ABI_ENTRY_H
    #define BRIDGE_SRC_ABI_ENTRY_H

    #include <string.h>

    #include "abi.h"

static inline const void *get_scratch(const module_t *m, uint64_t at,
    uint64_t len)
{
    if (at < m->scratch || len > m->scratch_len)
        return (nullptr);
    if (at - m->scratch > m->scratch_len - len)
        return (nullptr);
    return (m->priv + at);
}

static inline void mark_turn(module_t *m)
{
    if (m->mark_count && m->marks[m->mark_count - 1].turn == m->turn)
        return;
    m->marks[m->mark_count].turn = m->turn;
    m->marks[m->mark_count].at = m->queue_len;
    m->mark_count++;
}

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
    mark_turn(m);
    memcpy(at, ch, sizeof(*ch));
    if (value)
        memcpy(at + sizeof(*ch), value, ch->size);
    m->queue_len += need;
    return (0);
}

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
