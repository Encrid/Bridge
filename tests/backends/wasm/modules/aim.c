/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "aim.h"
#include "wasm_idle.h"

constexpr uint32_t ENTITIES = 10;

static uint32_t position;
static uint32_t aim;
static uint32_t bodies;

static uint32_t add_aims(uint32_t span, uint32_t *found, uint32_t *sum)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t table = bridge_span_table(span);
    uint64_t id;
    uint32_t r = 0;

    while (r < count) {
        id = bridge_get_entity(table, r);
        if (bridge_has(aim, id)) {
            *sum += BRIDGE_GET(aim_t, at, aim, (uint32_t)id, 0);
            (*found)++;
        }
        r++;
    }
    return (count);
}

BRIDGE_EXPORT(reached) int reached(void)
{
    uint32_t spans = bridge_count_spans(bodies);
    uint32_t rows = 0;
    uint32_t found = 0;
    uint32_t sum = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += add_aims(bridge_get_span(bodies, s), &found, &sum);
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "aims", rows, found);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "aimed", sum, 0);
    return (0);
}

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
    bodies = BRIDGE_ADD_SYSTEM("reached", "present", two, 2, nullptr, 0, nullptr, 0);
    populate();
    return (0);
}
