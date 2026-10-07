/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/wasm/mover.c -- writes game:Position. It owns the
** namespace, spawns the entities, and marks half of them.
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

constexpr uint32_t ENTITIES = 1000;

static uint32_t position;
static uint32_t marked;
static uint32_t moving;
static uint32_t counting;

/*
** move
**
** Adds one to the x of every position.
*/
BRIDGE_EXPORT(move) int move(void)
{
    uint32_t spans = bridge_count_spans(moving);
    uint32_t span;
    uint32_t count;
    uint32_t at;
    uint32_t s = 0;
    uint32_t r;

    while (s < spans) {
        span = bridge_get_span(moving, s, 1);
        count = bridge_count_rows(span);
        r = 0;
        while (r < count) {
            at = BRIDGE_ROW(position_t, bridge_get_base(span, 0), r, count);
            BRIDGE_SET(position_t, x, at, BRIDGE_GET(position_t, x, at) + 1.0f);
            r++;
        }
        s++;
    }
    return (0);
}

/*
** count
**
** Says what its own query matched: every table holding a position, marked
** or not.
*/
BRIDGE_EXPORT(count) int count(void)
{
    uint32_t spans = bridge_count_spans(counting);
    uint32_t rows = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += bridge_count_rows(bridge_get_span(counting, s, 1));
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "moved", spans, rows);
    return (0);
}

/*
** populate
**
** Spawns the entities: each with a position, and every other one marked.
*/
static void populate(void)
{
    position_t p = { 0.0f, 0.0f };
    marked_t m = {};
    uint64_t id;

    while (m.by < ENTITIES) {
        id = bridge_spawn();
        p.x = (float)m.by;
        BRIDGE_ATTACH(id, position, p);
        if (m.by % 2)
            BRIDGE_ATTACH(id, marked, m);
        m.by++;
    }
}

/*
** init
**
** Declares the two components and the two systems, and spawns the entities.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = position;
    moving = BRIDGE_ADD_SYSTEM("move", "update", terms, 1, nullptr, 0);
    terms[0].access = BRIDGE_READ;
    counting = BRIDGE_ADD_SYSTEM("count", "present", terms, 1, nullptr, 0);
    populate();
    return (0);
}
