/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nweigh/nweigh.c -- what the one storage layout there
** is costs at three kinds of work it was not chosen for, so that a storage
** layout made for one of them has a figure to be weighed beside. Each is
** done the way a module does it today.
**
**     flag     a component an entity always has and sometimes does not
**              use: a field says whether, and the loop tests it at every
**              row. One row in ON has it set
**     near     a value reached by entity id and not walked, as a reference
**     far      is followed. The ids in the order their entities lie, and in
**              no order
**     one      one value for the whole deployment: a component on one
**              entity, found through a query of one span, a million times
**     walk     the same number of rows read in order, which is what the
**              others are beside
**
** Built once for each ON: 1, 2, 16.
**
** stage11.md, *The baseline*
*/

#include "nweigh.h"
#include "native_idle.h"

uint32_t nweigh_flag;
uint32_t nweigh_link;
uint32_t nweigh_single;
static uint64_t ids[COUNT];
static uint32_t state = 2463534242u;

static uint32_t draw_number(void)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return (state);
}

/*
** fill_row
**
** Gives entity `i` its value, its flag, which one row in ON has set, and
** its link: its own id, and the id of an entity that was drawn.
*/
static void fill_row(const bridge_api_t *api, uint32_t i)
{
    v_t v = { 1 };
    f_t f = { draw_number() % ON == 0, 1 };
    l_t l = { ids[i], ids[draw_number() % COUNT] };

    BRIDGE_NATIVE_ATTACH(api, ids[i], nweigh_value, v);
    BRIDGE_NATIVE_ATTACH(api, ids[i], nweigh_flag, f);
    BRIDGE_NATIVE_ATTACH(api, ids[i], nweigh_link, l);
}

/*
** fill
**
** Spawns the entity that holds the one value, and then the entities that
** are walked, each with a value, a flag and a link.
*/
BRIDGE_NATIVE(fill)
{
    uint64_t holder = api->spawn(api);
    s_t s = { 1 };
    uint32_t i = 0;

    while (i < COUNT) {
        ids[i] = api->spawn(api);
        i++;
    }
    i = 0;
    while (i < COUNT) {
        fill_row(api, i);
        i++;
    }
    BRIDGE_NATIVE_ATTACH(api, holder, nweigh_single, s);
    return (0);
}

/*
** add_readers
**
** Registers the five systems that read, each with the one component it
** reads, and keeps their queries.
*/
static void add_readers(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ nweigh_value, BRIDGE_READ, BRIDGE_AND, 0 }};

    nweigh_walking = BRIDGE_NATIVE_ADD_SYSTEM(api, "walk", "walk", one, 1,
        nullptr, 0);
    one[0].slot = nweigh_flag;
    nweigh_flagged = BRIDGE_NATIVE_ADD_SYSTEM(api, "flag", "flag", one, 1,
        nullptr, 0);
    one[0].slot = nweigh_link;
    nweigh_linked = BRIDGE_NATIVE_ADD_SYSTEM(api, "near", "near", one, 1,
        nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "far", "far", one, 1, nullptr, 0);
    one[0].slot = nweigh_single;
    nweigh_which = BRIDGE_NATIVE_ADD_SYSTEM(api, "one", "one", one, 1,
        nullptr, 0);
}

/*
** init
**
** Declares the four components, and registers the system that fills them
** and the five that read.
*/
BRIDGE_NATIVE(init)
{
    int none = 0;
    uint32_t filled[4];

    nweigh_value = BRIDGE_NATIVE_COMPONENT(api, "st:V", v_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    nweigh_flag = BRIDGE_NATIVE_COMPONENT(api, "st:F", f_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    nweigh_link = BRIDGE_NATIVE_COMPONENT(api, "st:L", l_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    nweigh_single = BRIDGE_NATIVE_COMPONENT(api, "st:S", s_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    filled[0] = nweigh_value;
    filled[1] = nweigh_flag;
    filled[2] = nweigh_link;
    filled[3] = nweigh_single;
    BRIDGE_NATIVE_ADD_SYSTEM(api, "fill", "spawn", &none, 0, filled, 4);
    add_readers(api);
    return (0);
}
