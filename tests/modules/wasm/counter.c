/* Counts the rows of the shared component, and how many of the roster's ids are still alive:
 * what an unload left behind, seen by a module that stayed. */
#include "bridge-wasm.h"
#include "../roster.h"

static uint32_t position, roster, positions, rosters;

BRIDGE_EXPORT(count) int count()
{
    uint32_t spans = bridge_spans(positions), rows = 0, alive = 0, s, i;
    for (s = 0; s < spans; s ++) rows += bridge_rows(bridge_span(positions, s, 1));
    if (bridge_spans(rosters)) {
        uint32_t span = bridge_span(rosters, 0, 1);
        uint32_t at = BRIDGE_ROW(Roster, bridge_base(span, 0), 0, bridge_rows(span));
        for (i = 0; i < 5; i ++) {
            alive += bridge_alive(acc_ld_64(at + (uint32_t)offsetof(Roster, ids) + 8u * i));
        }
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "left", rows, alive);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    position = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_STRIP, BRIDGE_TABLE);
    roster = BRIDGE_COMPONENT("game:Roster", Roster, BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = position;
    positions = BRIDGE_SYSTEM("count", "update", terms, 1, nullptr, 0);
    terms[0].slot = roster;
    rosters = BRIDGE_SYSTEM("rosters", "present", terms, 1, nullptr, 0);
    return 0;
}

BRIDGE_EXPORT(rosters) int rosters_() { return 0; }
BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
