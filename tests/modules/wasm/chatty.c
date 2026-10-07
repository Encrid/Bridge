/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/chatty.c -- writes more lines in one phase than its
** ring holds.
*/

#include "bridge_wasm.h"
#include "wasm_idle.h"

constexpr uint32_t LINES = 100;

/*
** talk
**
** Writes a hundred lines, each with its number.
*/
BRIDGE_EXPORT(talk) int talk(void)
{
    uint32_t i = 0;

    while (i < LINES) {
        BRIDGE_LOG(BRIDGE_LOG_INFO, "line", i, 0);
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

    BRIDGE_ADD_SYSTEM("talk", "update", &none, 0, nullptr, 0);
    return (0);
}
