/* A column long enough to time: the loop of mover.c, over 200,000 rows. */
#include "bridge-wasm.h"
#include "game.h"

#define ENTITIES 200000

static uint32_t position, moving;

BRIDGE_EXPORT(move) int move(void)
{
    uint32_t spans = bridge_spans(moving), s, r;
    for (s = 0; s < spans; s ++) {
        uint32_t span = bridge_span(moving, s, 1);
        uint32_t count = bridge_rows(span), base = bridge_base(span, 0);
        for (r = 0; r < count; r ++) {
            uint32_t at = BRIDGE_ROW(Position, base, r, count);
            BRIDGE_SET(Position, x, at, BRIDGE_GET(Position, x, at) + 1.0f);
        }
    }
    return 0;
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term terms[1] = {{ 0, BRIDGE_INOUT, BRIDGE_AND, 0 }};
    uint32_t i;
    position = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_DELETE);
    terms[0].slot = position;
    moving = BRIDGE_SYSTEM("move", "update", terms, 1);
    /* zeroed: a value each would need a scratch arena of their sum */
    for (i = 0; i < ENTITIES; i ++) bridge_attach(bridge_spawn(), position, 0, 0);
    return 0;
}
