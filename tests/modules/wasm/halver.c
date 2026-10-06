/* Halves every game:Position.x, under a write grant, in the phase the mover adds to it. */
#include "bridge_wasm.h"
#include "../game.h"

static uint32_t position, halving;

BRIDGE_EXPORT(halve) int halve()
{
    uint32_t spans = bridge_count_spans(halving), s, r;
    for (s = 0; s < spans; s ++) {
        uint32_t span = bridge_get_span(halving, s, 1);
        uint32_t count = bridge_count_rows(span), base = bridge_get_base(span, 0);
        for (r = 0; r < count; r ++) {
            uint32_t at = BRIDGE_ROW(Position, base, r, count);
            BRIDGE_SET(Position, x, at, BRIDGE_GET(Position, x, at) * 0.5f);
        }
    }
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    position = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = position;
    halving = BRIDGE_ADD_SYSTEM("halve", "update", terms, 1, nullptr, 0);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
