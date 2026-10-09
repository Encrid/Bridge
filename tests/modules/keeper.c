/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/keeper.c -- declares a component it shares, one only
** it declares, and a roster of ids so that another module can ask after its
** entities once it is gone. Built once per on_unload policy, which
** PRIVATE_POLICY states.
**
** Ten entities carry position_t and private_t; five carry private_t alone.
** Its shutdown destroys the first of the ten: what a shutdown asks for is
** applied.
**
** lifetime.md, *What unload leaves behind*
*/

#include "test_offsets.h"
#include "roster.h"

constexpr uint32_t ENTITIES = 15;
constexpr uint32_t PLACED = 10;

static uint32_t position;
static uint32_t hidden;
static uint32_t roster;
static uint64_t first;

/*
** spawn_one
**
** Spawns entity `i` with its private component. One of the first ten is
** given a position as well; one of the last five has its id written into
** the roster `r`. Answers the id.
*/
static uint64_t spawn_one(uint32_t i, roster_t *r)
{
    private_t secret = { i };
    position_t p = { (float)i, 0.0f };
    uint64_t id = bridge_spawn();

    BRIDGE_ATTACH(id, hidden, secret);
    if (i < PLACED)
        BRIDGE_ATTACH(id, position, p);
    else
        r->ids[i - PLACED] = id;
    return (id);
}

/*
** init
**
** Declares the three components, spawns the fifteen entities and keeps the
** id of the first, and spawns one more that carries the roster.
*/
BRIDGE_EXPORT(init) int init(void)
{
    roster_t r = {};
    uint64_t id;
    uint32_t i = 0;

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    hidden = BRIDGE_COMPONENT("game:Private", private_t, PRIVATE_POLICY,
        BRIDGE_TABLE);
    roster = BRIDGE_COMPONENT("game:Roster", roster_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    while (i < ENTITIES) {
        id = spawn_one(i, &r);
        if (!i)
            first = id;
        i++;
    }
    BRIDGE_ATTACH(bridge_spawn(), roster, r);
    return (0);
}

/*
** shutdown
**
** Destroys the first entity.
*/
BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    bridge_destroy(first);
    return (0);
}

/*
** reload
**
** Does nothing.
*/
BRIDGE_EXPORT(reload) int reload(void)
{
    return (0);
}
