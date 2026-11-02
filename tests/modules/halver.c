/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

static uint32_t position;
static uint32_t halving;

static void halve_span(uint32_t span)
{
    uint32_t count = bridge_get_rows(span);
    uint32_t table = bridge_get_span_table(span);
    uint32_t r = 0;

    while (r < count) {
        BRIDGE_SET(position_t, x, position, table, r,
            BRIDGE_GET(position_t, x, position, table, r) * 0.5f);
        r++;
    }
}

BRIDGE_EXPORT(halve) int halve(void)
{
    uint32_t spans = bridge_get_span_count(halving);
    uint32_t s = 0;

    while (s < spans) {
        halve_span(bridge_get_span(halving, s));
        s++;
    }
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = position;
    halving = BRIDGE_ADD_SYSTEM("halve", "update", terms, 1, nullptr, 0,
        nullptr, 0);
    return (0);
}
