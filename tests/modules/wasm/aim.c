/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/aim.c -- a direct-indexed component on the wasm
** backend, reached from the rows of a table by each entity's id.
**
** storage.md, *Direct-indexed*
*/

#include "bridge_wasm.h"
#include "game.h"
#include "aim.h"
#include "wasm_idle.h"

constexpr uint32_t ENTITIES = 10;

static uint32_t position;
static uint32_t aim;
static uint32_t bodies;

/*
** add_aims
**
** Looks for the aim of each entity of one span, by its id. Adds one to
** `found` and the aim to `sum` for each that has one, and answers how many
** rows the span has.
*/
static uint32_t add_aims(uint32_t span, uint32_t cells, uint32_t *found,
    uint32_t *sum)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t entities = bridge_get_entities(span);
    uint32_t at;
    uint32_t r = 0;

    while (r < count) {
        at = bridge_get_cell(cells, bridge_get_entity(entities, r, count),
            sizeof(aim_t));
        if (at) {
            *sum += BRIDGE_GET(aim_t, at, at);
            (*found)++;
        }
        r++;
    }
    return (count);
}

/*
** reached
**
** Walks the table of positions, and says how many rows it has, how many of
** them have an aim, and the sum of those aims.
*/
BRIDGE_EXPORT(reached) int reached(void)
{
    uint32_t spans = bridge_count_spans(bodies);
    uint32_t cells = bridge_get_cells(aim);
    uint32_t rows = 0;
    uint32_t found = 0;
    uint32_t sum = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += add_aims(bridge_get_span(bodies, s, 2), cells, &found, &sum);
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "aims", rows, found);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "aimed", sum, 0);
    return (0);
}

/*
** populate
**
** Spawns the entities: each with a position, and every other one with an
** aim.
*/
static void populate(void)
{
    position_t p = { 0.0f, 0.0f };
    aim_t value = {};
    uint64_t id;

    while (value.at < ENTITIES) {
        id = bridge_spawn();
        p.x = (float)value.at;
        BRIDGE_ATTACH(id, position, p);
        if (value.at % 2 == 0)
            BRIDGE_ATTACH(id, aim, value);
        value.at++;
    }
}

/*
** init
**
** Declares the two components and the system, and spawns the entities.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t two[2] = {
        { 0, BRIDGE_READ, BRIDGE_AND, 0 },
        { 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 },
    };

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    aim = BRIDGE_COMPONENT("game:Aim", aim_t, BRIDGE_DELETE,
        BRIDGE_DIRECT_INDEXED);
    two[0].slot = position;
    two[1].slot = aim;
    bodies = BRIDGE_ADD_SYSTEM("reached", "present", two, 2, nullptr, 0);
    populate();
    return (0);
}
