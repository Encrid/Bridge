/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/module_share.c -- a module's share of the persistent arena, given
** back.
**
** strings.md, *It is given back when nothing points into it*
*/

#include "module.h"

/*
** module_give_share
**
** Gives a module's share of the persistent arena back to it.
*/
void module_give_share(bridge_t *b, const module_t *m)
{
    const range_t *own = m->owned;
    uint32_t i = 0;

    while (i < m->own_count) {
        region_give(&b->arena_space, own[i].at - b->arena, own[i].len);
        i++;
    }
    b->arena_ranges -= m->own_count;
}

static uint64_t get_share_size(const module_t *m)
{
    uint64_t bytes = 0;
    uint32_t i = 0;

    while (i < m->own_count) {
        bytes += m->owned[i].len;
        i++;
    }
    return (bytes);
}

/*
** is_share_held
**
** Answers whether a row that is staying holds a string in a module's share.
** A share that stays is said, with what holds it: a later load may be
** refused for the room it keeps.
**
** strings.md, *It is given back when nothing points into it*
*/
static bool is_share_held(bridge_t *b, const module_t *m)
{
    say_t about = { nullptr, nullptr, BRIDGE_WARN };
    unsigned long long bytes = get_share_size(m);
    bool held = false;
    uint32_t rows;
    uint32_t i = 0;

    while (i < b->type_count) {
        rows = module_count_pointing(b, m, b->types[i]);
        if (rows)
            say_about(b, &about, "module '%s' is unloaded and its share of "
                "the persistent arena, %llu bytes, stays: '%s' holds a "
                "string in it, in this many rows: %u", m->identity, bytes,
                b->types[i]->name, rows);
        held = held || rows;
        i++;
    }
    return (held);
}

/*
** module_settle_share
**
** Gives a module's share of the persistent arena back at its unload, unless
** a row that is staying holds a string in it.
*/
void module_settle_share(bridge_t *b, module_t *m)
{
    if (!m->own_count || is_share_held(b, m)) {
        config_keep(b, m);
        return;
    }
    module_give_share(b, m);
    config_forget(b, m);
}
