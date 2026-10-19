/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "roster.h"

constexpr uint32_t ENTITIES = 15;
constexpr uint32_t PLACED = 10;

static uint32_t position;
static uint32_t hidden;
static uint32_t roster;
static uint64_t first;

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

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    bridge_destroy(first);
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    return (0);
}
