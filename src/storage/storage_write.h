/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_WRITE_H
    #define BRIDGE_SRC_STORAGE_WRITE_H

    #include "storage_value.h"

[[gnu::always_inline]] static inline bool is_written(const storage_t *r,
    const change_t *ch)
{
    uint32_t index = (uint32_t)ch->id;
    uint8_t folded = index < r->high ? r->folded[index] : 0;

    if (ch->op != CHANGE_ATTACH || !(folded & TOUCHED))
        return (false);
    if (folded & (DESTROYED | DROPPED))
        return (false);
    return (r->places[index].generation == (uint32_t)(ch->id >> 32));
}

[[gnu::always_inline]] static inline void write_value(storage_t *r,
    module_t *m, const change_t *ch, const uint8_t *value)
{
    uint32_t index = (uint32_t)ch->id;
    uint8_t folded = index < r->high ? r->folded[index] : 0;
    uint32_t to = 0;

    if (ch->op == CHANGE_SKIPPED)
        return;
    if (folded & DROPPED
        && r->places[index].generation == (uint32_t)(ch->id >> 32))
        m->left[(folded & SHORT) != 0]++;
    if (is_written(r, ch))
        to = get_value(r, index, ch->c->type);
    if (to)
        copy_bytes(r->w->shared + to, value, ch->size);
}

#endif
