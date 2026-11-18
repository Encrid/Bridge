/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "lane.h"
#include "offsets_idle.h"

constexpr uint32_t LANES = 3;

static uint32_t values[LANES];
static uint32_t summing[LANES];
static uint64_t total;

static uint64_t hashed(uint64_t id, uint64_t v)
{
    return ((id * 0x9E37'79B9'7F4A'7C15ull ^ v) * 0xBF58'476D'1CE4'E5B9ull);
}

static void add_lane(uint32_t lane)
{
    uint32_t spans = bridge_get_span_count(summing[lane]);
    uint32_t span;
    uint32_t count;
    uint32_t table;
    uint32_t s = 0;
    uint32_t r;

    while (s < spans) {
        span = bridge_get_span(summing[lane], s);
        count = bridge_get_rows(span);
        table = bridge_get_span_table(span);
        r = 0;
        while (r < count) {
            total += hashed(bridge_get_entity(table, r),
                BRIDGE_GET(lane_value_t, v, values[lane], table, r));
            r++;
        }
        s++;
    }
}

BRIDGE_EXPORT(sum1) int sum1(void)
{
    add_lane(0);
    return (0);
}

BRIDGE_EXPORT(sum2) int sum2(void)
{
    add_lane(1);
    return (0);
}

BRIDGE_EXPORT(sum3) int sum3(void)
{
    add_lane(2);
    return (0);
}

BRIDGE_EXPORT(tell) int tell(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "bit", (uint32_t)(total >> 32),
        (uint32_t)total);
    total = 0;
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    int none = 0;

    total = 0;
    values[0] = BRIDGE_COMPONENT("lane1:Value", lane_value_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    values[1] = BRIDGE_COMPONENT("lane2:Value", lane_value_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    values[2] = BRIDGE_COMPONENT("lane3:Value", lane_value_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = values[0];
    summing[0] = BRIDGE_ADD_SYSTEM("sum1", "update", terms, 1, nullptr, 0,
        nullptr, 0);
    terms[0].slot = values[1];
    summing[1] = BRIDGE_ADD_SYSTEM("sum2", "update", terms, 1, nullptr, 0,
        nullptr, 0);
    terms[0].slot = values[2];
    summing[2] = BRIDGE_ADD_SYSTEM("sum3", "update", terms, 1, nullptr, 0,
        nullptr, 0);
    BRIDGE_ADD_SYSTEM("tell", "update", &none, 0, nullptr, 0, summing,
        LANES);
    return (0);
}
