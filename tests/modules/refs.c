/* Follows ids it kept, across two batches of destroys. Every destroy moves the last row into
 * the hole, which no event names: the survivors must still be found, and the dead refused. */
#include "bridge-wasm.h"
#include "game.h"

#define ENTITIES 100

static uint32_t position, round;
static uint64_t ids[ENTITIES];

BRIDGE_EXPORT(cull) int cull(void)
{
    uint32_t i;
    for (i = round; i < ENTITIES && round < 2; i += 3) bridge_destroy(ids[i]);
    round ++;
    return 0;
}

BRIDGE_EXPORT(check) int check(void)
{
    uint32_t i, right = 0, wrong = 0;
    for (i = 0; i < ENTITIES; i ++) {
        uint32_t at = bridge_ref(ids[i], position, sizeof(Position));
        int dead = i % 3 < round && i % 3 < 2;
        if (dead ? at == 0 : at && BRIDGE_GET(Position, x, at) == (float)i) right ++;
        else wrong ++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "mirror", right, wrong);
    return 0;
}

BRIDGE_EXPORT(init) int init(void)
{
    uint32_t i;
    position = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_DELETE);
    BRIDGE_SYSTEM("cull", "update", nullptr, 0);
    BRIDGE_SYSTEM("check", "present", nullptr, 0);
    for (i = 0; i < ENTITIES; i ++) {
        Position p = { (float)i, 0.0f };
        ids[i] = bridge_spawn();
        BRIDGE_ATTACH(ids[i], position, p);
    }
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown(void) { return 0; }
BRIDGE_EXPORT(reload) int reload(void) { return 0; }
