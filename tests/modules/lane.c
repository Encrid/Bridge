/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "lane.h"
#include "offsets_idle.h"

constexpr uint32_t FIRST = 64;
constexpr uint32_t BORN = 4;

static uint32_t value;
static uint32_t seen;
static uint32_t stepping;
static uint32_t hearing;
static uint32_t frame;

static uint64_t mixed(uint64_t v)
{
    return (v * 6'364'136'223'846'793'005ull
        + (uint64_t)LANE * 1'442'695'040'888'963'407ull + SEED);
}

static void born(uint64_t from)
{
    lane_value_t v = { mixed(from) };
    uint64_t id = bridge_spawn();

    BRIDGE_ATTACH(id, value, v);
}

static void step_span(uint32_t span)
{
    uint32_t count = bridge_get_rows(span);
    uint32_t table = bridge_get_span_table(span);
    uint64_t v;
    uint32_t r = 0;

    while (r < count) {
        v = mixed(BRIDGE_GET(lane_value_t, v, value, table, r));
        BRIDGE_SET(lane_value_t, v, value, table, r, v);
        if (v >> 61 == 0)
            bridge_destroy(bridge_get_entity(table, r));
        r++;
    }
}

BRIDGE_EXPORT(step) int step(void)
{
    uint32_t spans = bridge_get_span_count(stepping);
    uint32_t s = 0;
    uint32_t i = 0;

    while (s < spans) {
        step_span(bridge_get_span(stepping, s));
        s++;
    }
    while (i < BORN) {
        born((uint64_t)frame * BORN + i);
        i++;
    }
    frame++;
    return (0);
}

BRIDGE_EXPORT(hear) int hear(void)
{
    uint32_t count = bridge_get_trigger_count(hearing);
    lane_seen_t s = { frame };
    uint32_t i = 0;

    while (i < count) {
        BRIDGE_ATTACH(bridge_get_trigger_id(bridge_get_trigger(hearing, i)),
            seen, s);
        i++;
    }
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    uint32_t i = 0;

    frame = 0;
    value = BRIDGE_COMPONENT(LANE_SPACE ":Value", lane_value_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    seen = BRIDGE_COMPONENT(LANE_SPACE ":Seen", lane_seen_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = value;
    stepping = BRIDGE_ADD_SYSTEM("step", "update", terms, 1, &value, 1,
        nullptr, 0);
    terms[0].access = BRIDGE_READ;
    hearing = BRIDGE_OBSERVER("hear", BRIDGE_ON_ENTER, terms, 1, &seen, 1,
        nullptr, 0);
    while (i < FIRST) {
        born(i);
        i++;
    }
    return (0);
}
