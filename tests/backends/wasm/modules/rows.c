/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

constexpr uint32_t ENTITIES = 200'000;

static uint32_t position;
static uint32_t moving;

static void move_span(uint32_t span)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t table = bridge_span_table(span);
    uint32_t r = 0;

    while (r < count) {
        BRIDGE_SET(position_t, x, position, table, r,
            BRIDGE_GET(position_t, x, position, table, r) + 1.0f);
        r++;
    }
}

BRIDGE_EXPORT(move) int move(void)
{
    uint32_t spans = bridge_count_spans(moving);
    uint32_t s = 0;

    while (s < spans) {
        move_span(bridge_get_span(moving, s));
        s++;
    }
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    uint32_t i = 0;

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    terms[0].slot = position;
    moving = BRIDGE_ADD_SYSTEM("move", "update", terms, 1, nullptr, 0, nullptr, 0);
    while (i < ENTITIES) {
        bridge_attach(bridge_spawn(), position, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}
