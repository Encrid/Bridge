/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nstress/nstress_work.c -- the changes nstress asks
** for, drawn by a generator it seeds itself, and written into its account.
*/

#include "nstress.h"

static uint32_t state = SEED;

static uint32_t draw_number(void)
{
    state = state * 1'664'525u + 1'013'904'223u;
    return (state >> 8);
}

/*
** set_value
**
** Gives the entity of `slot` a drawn value for component `kind`: an attach,
** or a set where it has the component already.
*/
static void set_value(const bridge_api_t *api, uint32_t slot, uint32_t kind)
{
    v_t value = { draw_number() };

    BRIDGE_NATIVE_ATTACH(api, nstress_ids[slot], nstress_kinds[kind], value);
    nstress_has[slot] |= (uint8_t)(1u << kind);
    nstress_values[slot][kind] = value.v;
}

/*
** make_entity
**
** Spawns the entity of `slot`, with one to three components that are
** drawn.
*/
static void make_entity(const bridge_api_t *api, uint32_t slot)
{
    uint32_t k;

    nstress_ids[slot] = api->spawn(api);
    nstress_has[slot] = 0;
    k = 1 + draw_number() % 3;
    while (k) {
        set_value(api, slot, draw_number() % KINDS);
        k--;
    }
}

/*
** change_entity
**
** Changes the entity of `slot` by what was drawn: 15 times in 100 it is
** destroyed, 55 times it is given a value for component `kind`, and 30
** times that component is detached.
*/
static void change_entity(const bridge_api_t *api, uint32_t slot,
    uint32_t what, uint32_t kind)
{
    if (what < 15) {
        api->destroy(api, nstress_ids[slot]);
        nstress_ids[slot] = 0;
    } else if (what < 70) {
        set_value(api, slot, kind);
    } else {
        api->detach(api, nstress_ids[slot], nstress_kinds[kind]);
        nstress_has[slot] &= (uint8_t)~(1u << kind);
    }
}

/*
** work
**
** Asks for twenty thousand changes, each of an entity that is drawn: one
** that does not exist is made, and one that does is changed.
*/
BRIDGE_NATIVE(work)
{
    uint32_t slot;
    uint32_t what;
    uint32_t kind;
    uint32_t n = 0;

    while (n < CHANGES) {
        slot = draw_number() % SLOTS;
        what = draw_number() % 100;
        kind = draw_number() % KINDS;
        if (!nstress_ids[slot])
            make_entity(api, slot);
        else
            change_entity(api, slot, what, kind);
        n++;
    }
    return (0);
}
