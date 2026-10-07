/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/flood.c -- spawns until the entity ceiling, and past
** it.
*/

#include "bridge_wasm.h"
#include "wasm_idle.h"

constexpr uint32_t SPAWNS = 1'000'000;

/*
** flood
**
** Spawns a million entities.
*/
BRIDGE_EXPORT(flood) int flood(void)
{
    uint32_t i = 0;

    while (i < SPAWNS) {
        bridge_spawn();
        i++;
    }
    return (0);
}

/*
** init
**
** Registers the system.
*/
BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("flood", "update", &none, 0, nullptr, 0);
    return (0);
}
