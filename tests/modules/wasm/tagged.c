/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/tagged.c -- a tag, declared and attached by a wasm
** module, and a system that walks the entities that carry it.
**
** storage.md, *A tag*
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

static uint32_t position;
static uint32_t frozen;
static uint32_t counting;

/*
** count
**
** Says how many rows carry the tag, and in how many spans the place of its
** value is BRIDGE_CARRIED.
*/
BRIDGE_EXPORT(count) int count(void)
{
    uint32_t spans = bridge_count_spans(counting);
    uint32_t rows = 0;
    uint32_t carried = 0;
    uint32_t span;
    uint32_t s = 0;

    while (s < spans) {
        span = bridge_get_span(counting, s, 2);
        rows += bridge_count_rows(span);
        carried += bridge_get_base(span, 1) == BRIDGE_CARRIED;
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "frozen", rows, carried);
    return (0);
}

/*
** populate
**
** Spawns three entities with a position, and freezes two of them.
*/
static void populate(void)
{
    position_t p = { 0.0f, 0.0f };
    uint64_t id;
    uint32_t i = 0;

    while (i < 3) {
        id = bridge_spawn();
        BRIDGE_ATTACH(id, position, p);
        if (i)
            bridge_attach(id, frozen, BRIDGE_ZEROED);
        i++;
    }
}

/*
** init
**
** Declares the position and the tag, registers the system that reads both,
** and spawns the entities.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[2] = {
        { 0, BRIDGE_READ, BRIDGE_AND, 0 },
        { 0, BRIDGE_READ, BRIDGE_AND, 0 }
    };

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    frozen = BRIDGE_DECLARE_TAG("game:Frozen", BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = position;
    terms[1].slot = frozen;
    counting = BRIDGE_ADD_SYSTEM("count", "update", terms, 2, nullptr, 0);
    populate();
    return (0);
}
