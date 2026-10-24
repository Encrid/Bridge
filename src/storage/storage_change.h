/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_CHANGE_H
    #define BRIDGE_SRC_STORAGE_CHANGE_H

    #include "storage_stake.h"
    #include "storage_tell.h"

enum fold : uint32_t {
    FOLD_SKIP,
    FOLD_APPLY,
    FOLD_UNHANDED
};

[[gnu::always_inline]] static inline enum fold foldable(const storage_t *r,
    const change_t *ch)
{
    uint32_t index = (uint32_t)ch->id;

    if (ch->op != CHANGE_DESTROY && ch->c->type->kept == BRIDGE_SINGLETON)
        return (FOLD_APPLY);
    if (!is_alive(r, ch->id) || r->folded[index] & DESTROYED)
        return (FOLD_SKIP);
    if (ch->op == CHANGE_DESTROY || !is_held(r, index))
        return (FOLD_APPLY);
    return (FOLD_UNHANDED);
}

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

[[gnu::always_inline]] static inline void fold_change(storage_t *r,
    module_t *m, const change_t *ch)
{
    uint32_t index = (uint32_t)ch->id;
    uint32_t type = ch->op == CHANGE_DESTROY ? 0 : ch->c->type->id;
    bool attach = ch->op == CHANGE_ATTACH;
    enum fold verdict = foldable(r, ch);

    if (verdict == FOLD_UNHANDED)
        m->unhanded++;
    if (verdict != FOLD_APPLY)
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
