/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "roster.h"
#include "wasm_idle.h"

constexpr uint32_t LISTED = 5;

static uint32_t position;
static uint32_t roster;
static uint32_t counting;
static uint32_t listing;

static uint32_t count_alive(void)
{
    uint32_t span;
    uint32_t table;
    uint32_t alive = 0;
    uint32_t i = 0;

    if (!bridge_count_spans(listing))
        return (0);
    span = bridge_get_span(listing, 0);
    table = bridge_span_table(span);
    while (i < LISTED) {
        alive += bridge_is_alive(acc_ld_64(roster, BRIDGE_AT(table, 0),
            (uint32_t)offsetof(roster_t, ids) + 8u * i));
        i++;
    }
    return (alive);
}

BRIDGE_EXPORT(count) int count(void)
{
    uint32_t spans = bridge_count_spans(counting);
    uint32_t rows = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += bridge_count_rows(bridge_get_span(counting, s));
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "left", rows, count_alive());
    return (0);
}

BRIDGE_EXPORT(rosters) int rosters(void)
{
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    roster = BRIDGE_COMPONENT("game:Roster", roster_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = position;
    counting = BRIDGE_ADD_SYSTEM("count", "update", terms, 1, nullptr, 0, nullptr, 0);
    terms[0].slot = roster;
    listing = BRIDGE_ADD_SYSTEM("rosters", "present", terms, 1, nullptr, 0, nullptr, 0);
    return (0);
}
