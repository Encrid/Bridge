/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "roster.h"
#include "offsets_idle.h"
#include "on_destroy.h"

constexpr uint32_t LISTED = 5;

#ifdef HUSK
constexpr uint32_t HANDLES = 1;
#else
constexpr uint32_t HANDLES = 0;
#endif

static uint32_t position;
static uint32_t roster;
static uint32_t handle;
static uint32_t counting;
static uint32_t listing;
static bool handed;

static uint64_t get_listed(uint32_t table, uint32_t i)
{
    return (acc_ld_64(roster, table, 0,
        (uint32_t)offsetof(roster_t, ids) + 8u * i));
}

static uint32_t count_alive(void)
{
    uint32_t table;
    uint32_t alive = 0;
    uint32_t i = 0;

    if (!bridge_count_spans(listing))
        return (0);
    table = bridge_span_table(bridge_get_span(listing, 0));
    while (i < LISTED) {
        alive += bridge_is_alive(get_listed(table, i));
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
    private_t mark = { 1 };
    uint32_t table;
    uint32_t i = 0;

    if (!HANDLES || handed || !bridge_count_spans(listing))
        return (0);
    table = bridge_span_table(bridge_get_span(listing, 0));
    while (i < LISTED) {
        BRIDGE_ATTACH(get_listed(table, i), handle, mark);
        i++;
    }
    handed = true;
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    roster = BRIDGE_COMPONENT("game:Roster", roster_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    if (HANDLES)
        handle = BRIDGE_DECLARE_AS(BRIDGE_STRUCT, "view:Handle", private_t,
            BRIDGE_STRIP, BRIDGE_TABLE, ON_DESTROY);
    terms[0].slot = position;
    counting = BRIDGE_ADD_SYSTEM("count", "update", terms, 1, nullptr, 0,
        nullptr, 0);
    terms[0].slot = roster;
    listing = BRIDGE_ADD_SYSTEM("rosters", "present", terms, 1, &handle,
        HANDLES, nullptr, 0);
    return (0);
}
