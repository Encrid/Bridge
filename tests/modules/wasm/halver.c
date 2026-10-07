/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/halver.c -- halves every game:Position.x, under a
** write grant, in the phase the mover adds to it.
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

static uint32_t position;
static uint32_t halving;

/*
** halve_span
**
** Halves the x of every position of one span.
*/
static void halve_span(uint32_t span)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t base = bridge_get_base(span, 0);
    uint32_t at;
    uint32_t r = 0;

    while (r < count) {
        at = BRIDGE_ROW(position_t, base, r, count);
        BRIDGE_SET(position_t, x, at, BRIDGE_GET(position_t, x, at) * 0.5f);
        r++;
    }
}

/*
** halve
**
** Halves the x of every position.
*/
BRIDGE_EXPORT(halve) int halve(void)
{
    uint32_t spans = bridge_count_spans(halving);
    uint32_t s = 0;

    while (s < spans) {
        halve_span(bridge_get_span(halving, s, 1));
        s++;
    }
    return (0);
}

/*
** init
**
** Declares the position, and the system that writes it.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = position;
    halving = BRIDGE_ADD_SYSTEM("halve", "update", terms, 1, nullptr, 0);
    return (0);
}
