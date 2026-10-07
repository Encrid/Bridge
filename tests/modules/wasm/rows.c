/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/rows.c -- a column long enough to time: the loop of
** mover.c, over 200,000 rows.
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

constexpr uint32_t ENTITIES = 200'000;

static uint32_t position;
static uint32_t moving;

/*
** move_span
**
** Adds one to the x of every position of one span.
*/
static void move_span(uint32_t span)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t base = bridge_get_base(span, 0);
    uint32_t at;
    uint32_t r = 0;

    while (r < count) {
        at = BRIDGE_ROW(position_t, base, r, count);
        BRIDGE_SET(position_t, x, at, BRIDGE_GET(position_t, x, at) + 1.0f);
        r++;
    }
}

/*
** move
**
** Adds one to the x of every position.
*/
BRIDGE_EXPORT(move) int move(void)
{
    uint32_t spans = bridge_count_spans(moving);
    uint32_t s = 0;

    while (s < spans) {
        move_span(bridge_get_span(moving, s, 1));
        s++;
    }
    return (0);
}

/*
** init
**
** Declares the position and the system, and spawns the entities. Each
** position is attached zeroed: a value each would need a scratch arena of
** their sum.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    uint32_t i = 0;

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    terms[0].slot = position;
    moving = BRIDGE_ADD_SYSTEM("move", "update", terms, 1, nullptr, 0);
    while (i < ENTITIES) {
        bridge_attach(bridge_spawn(), position, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}
