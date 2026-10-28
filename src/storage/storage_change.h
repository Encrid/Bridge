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
    FOLD_UNHANDED,
    FOLD_HUSK
};

[[gnu::always_inline]] static inline enum fold foldable(const storage_t *r,
    const change_t *ch)
{
    uint32_t index = (uint32_t)ch->id;

    if (ch->op != CHANGE_DESTROY && ch->c->type->kept == BRIDGE_SINGLETON)
        return (FOLD_APPLY);
    if (!is_alive(r, ch->id) || r->folded[index] & DESTROYED)
        return (FOLD_SKIP);
    if (ch->op != CHANGE_DETACH && is_husk(r, index))
        return (FOLD_HUSK);
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

[[gnu::always_inline]] static inline enum fold fold_change(storage_t *r,
    module_t *m, const change_t *ch)
{
    uint32_t index = (uint32_t)ch->id;
    uint32_t type = ch->op == CHANGE_DESTROY ? 0 : ch->c->type->id;
    bool attach = ch->op == CHANGE_ATTACH;
    enum fold verdict = foldable(r, ch);

    if (verdict == FOLD_UNHANDED)
        m->unhanded++;
    if (verdict != FOLD_APPLY)
        return (verdict);
    begin_entity(r, index);
    if (ch->op == CHANGE_DESTROY) {
        fold_destroy(r, m, index);
        return (verdict);
    }
    set_write(get_work(r, index), type, attach);
    if (r->observer_count)
        set_write(get_attached(r, index), type, attach);
    if (!attach)
        end_husk(r, index);
    return (verdict);
}

static inline void skip_change(module_t *m, uint32_t at)
{
    enum change_op op = CHANGE_SKIPPED;

    memcpy(m->queue + at + offsetof(change_t, op), &op, sizeof(op));
}

[[gnu::always_inline]] static inline void fold_run(storage_t *r, module_t *m,
    uint32_t at, uint32_t end)
{
    change_t ch;

    while (at < end) {
        memcpy(&ch, m->queue + at, sizeof(ch));
        if (fold_change(r, m, &ch) == FOLD_HUSK)
            skip_change(m, at);
        at += (uint32_t)sizeof(ch) + ((ch.size + 7u) & ~7u);
    }
}

#endif
