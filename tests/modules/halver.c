/* Halves every game:Position.x, under a write grant, in the phase the mover adds to it. */
#include "bridge-wasm.h"
#include "game.h"

static uint32_t position, halving;

BRIDGE_EXPORT(halve) int halve(void)
{
    uint32_t spans = bridge_spans(halving), s, r;
    for (s = 0; s < spans; s ++) {
        uint32_t span = bridge_span(halving, s, 1);
        uint32_t count = bridge_rows(span), base = bridge_base(span, 0);
        for (r = 0; r < count; r ++) {
            uint32_t at = BRIDGE_ROW(Position, base, r, count);
            BRIDGE_SET(Position, x, at, BRIDGE_GET(Position, x, at) * 0.5f);
        }
    }
    return 0;
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term terms[1] = {{ 0, BRIDGE_INOUT, BRIDGE_AND, 0 }};
    position = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_STRIP);
    terms[0].slot = position;
    halving = BRIDGE_SYSTEM("halve", "update", terms, 1);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown(void) { return 0; }
BRIDGE_EXPORT(reload) int reload(void) { return 0; }
