/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/wasm/refs.c -- follows ids it kept, across two batches of
** destroys. Every destroy moves the last row into the hole, which no event
** names: the survivors must still be found, and the dead refused.
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

constexpr uint32_t ENTITIES = 100;

static uint32_t position;
static uint32_t round;
static uint64_t ids[ENTITIES];

/*
** cull
**
** Destroys every third entity, from the first on its first pass and from
** the second on its second pass, and none after that.
*/
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

/*
** is_mirrored
**
** Answers whether id `i` leads where it must: nowhere when its entity was
** destroyed, and to the position it was given when it was not.
*/
static bool is_mirrored(uint32_t i)
{
    uint32_t at = bridge_get_ref(ids[i], position, sizeof(position_t));
    bool dead = i % 3 < round && i % 3 < 2;

    if (dead)
        return (at == 0);
    return (at && BRIDGE_GET(position_t, x, at) == (float)i);
}

/*
** check
**
** Says how many ids lead where they must, and how many do not.
*/
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

/*
** init
**
** Declares the position and the two systems, which state no term, and
** spawns the entities, keeping each id.
*/
BRIDGE_EXPORT(init) int init(void)
{
    position_t p = { 0.0f, 0.0f };
    uint32_t i = 0;

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    BRIDGE_ADD_SYSTEM("cull", "update", nullptr, 0, nullptr, 0);
    BRIDGE_ADD_SYSTEM("check", "present", nullptr, 0, nullptr, 0);
    while (i < ENTITIES) {
        p.x = (float)i;
        ids[i] = bridge_spawn();
        BRIDGE_ATTACH(ids[i], position, p);
        i++;
    }
    return (0);
}
