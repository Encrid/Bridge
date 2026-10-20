/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

constexpr uint32_t ENTITIES = 100;

static uint32_t position;
static uint32_t round;
static uint64_t ids[ENTITIES];

BRIDGE_EXPORT(cull) int cull(void)
{
    uint32_t i = round;

    while (i < ENTITIES && round < 2) {
        bridge_destroy(ids[i]);
        i += 3;
    }
    round++;
    return (0);
}

static bool is_mirrored(uint32_t i)
{
    uint64_t e = bridge_find(ids[i]);
    bool dead = i % 3 < round && i % 3 < 2;

    if (dead)
        return (!bridge_is_alive(ids[i]));
    return (bridge_is_alive(ids[i])
        && BRIDGE_GET(position_t, x, position, bridge_entity_table(e),
            bridge_entity_row(e)) == (float)i);
}

BRIDGE_EXPORT(check) int check(void)
{
    uint32_t right = 0;
    uint32_t wrong = 0;
    uint32_t i = 0;

    while (i < ENTITIES) {
        if (is_mirrored(i))
            right++;
        else
            wrong++;
        i++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "mirror", right, wrong);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    position_t p = { 0.0f, 0.0f };
    uint32_t i = 0;

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    BRIDGE_ADD_SYSTEM("cull", "update", nullptr, 0, nullptr, 0, nullptr, 0);
    BRIDGE_ADD_SYSTEM("check", "present", nullptr, 0, nullptr, 0, nullptr, 0);
    while (i < ENTITIES) {
        p.x = (float)i;
        ids[i] = bridge_spawn();
        BRIDGE_ATTACH(ids[i], position, p);
        i++;
    }
    return (0);
}
