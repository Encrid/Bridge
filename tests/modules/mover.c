/* Writes game:Position. It owns the namespace, spawns the entities, and marks half of them. */
#include "bridge-wasm.h"
#include "game.h"

#define ENTITIES 1000

static uint32_t position, marked, moving, counting;

BRIDGE_EXPORT(move) int move()
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

/* what its own query matched: every table holding a Position, marked or not */
BRIDGE_EXPORT(count) int count()
{
    uint32_t spans = bridge_spans(counting), rows = 0, s;
    for (s = 0; s < spans; s ++) rows += bridge_rows(bridge_span(counting, s, 1));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "moved", spans, rows);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term terms[1] = {{ 0, BRIDGE_INOUT, BRIDGE_AND, 0 }};
    uint32_t i;
    position = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_DELETE);
    marked = BRIDGE_COMPONENT("game:Marked", Marked, BRIDGE_STRIP);
    terms[0].slot = position;
    moving = BRIDGE_SYSTEM("move", "update", terms, 1);
    terms[0].access = BRIDGE_IN;
    counting = BRIDGE_SYSTEM("count", "present", terms, 1);
    for (i = 0; i < ENTITIES; i ++) {
        uint64_t id = bridge_spawn();
        Position p = { (float)i, 0.0f };
        BRIDGE_ATTACH(id, position, p);
        if (i % 2) {
            Marked m = { i };
            BRIDGE_ATTACH(id, marked, m);
        }
    }
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
