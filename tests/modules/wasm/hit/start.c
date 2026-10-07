/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/hit/start.c -- a component kept in a sparse set, on
** the wasm backend: what the hit module declares, and what it spawns.
**
** storage.md, *A sparse set*
*/

#include "bridge_wasm.h"
#include "game.h"
#include "hit.h"
#include "wasm_idle.h"

constexpr uint32_t ENTITIES = 10;

uint32_t position;
uint32_t hit;
uint32_t hits;
uint32_t bodies;

/*
** populate
**
** Spawns the entities: each with a position, and every third one with a
** hit.
*/
static void populate(void)
{
    position_t p = { 0.0f, 0.0f };
    hit_t h = {};
    uint64_t id;

    while (h.by < ENTITIES) {
        id = bridge_spawn();
        p.x = (float)h.by;
        BRIDGE_ATTACH(id, position, p);
        if (h.by % 3 == 0)
            BRIDGE_ATTACH(id, hit, h);
        h.by++;
    }
}

/*
** init
**
** Declares the two components, a system over the set and a system over the
** table with the set as an optional term, and spawns the entities.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {
        { 0, BRIDGE_READ, BRIDGE_AND, 0 },
        { 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
    };

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    hit = BRIDGE_COMPONENT("game:Hit", hit_t, BRIDGE_DELETE,
        BRIDGE_SPARSE_SET);
    one[0].slot = hit;
    hits = BRIDGE_ADD_SYSTEM("walked", "update", one, 1, nullptr, 0);
    two[0].slot = position;
    two[1].slot = hit;
    bodies = BRIDGE_ADD_SYSTEM("reached", "present", two, 2, nullptr, 0);
    populate();
    return (0);
}
