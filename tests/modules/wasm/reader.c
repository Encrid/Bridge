/* Reads game:Position under a grant, over the marked entities only, and says what it saw. */
#include "bridge-wasm.h"
#include "../game.h"

static uint32_t position, marked, summing;

BRIDGE_EXPORT(sum) int sum()
{
    uint32_t spans = bridge_spans(summing), rows = 0, s, r;
    float total = 0.0f;
    for (s = 0; s < spans; s ++) {
        uint32_t span = bridge_span(summing, s, 2);
        uint32_t count = bridge_rows(span), base = bridge_base(span, 0);
        for (r = 0; r < count; r ++) {
            total += BRIDGE_GET(Position, x, BRIDGE_ROW(Position, base, r, count));
        }
        rows += count;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "saw", spans, rows);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "sum", (uint32_t)total, 0);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term_t terms[2] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }, { 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    position = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_STRIP, BRIDGE_TABLE);
    marked = BRIDGE_COMPONENT("game:Marked", Marked, BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = position;
    terms[1].slot = marked;
    summing = BRIDGE_SYSTEM("sum", "present", terms, 2, nullptr, 0);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
