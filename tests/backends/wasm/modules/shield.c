/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "shield.h"
#include "wasm_idle.h"

constexpr uint32_t ENTITIES = 10;

static uint32_t shield;
static uint32_t lowering;
static uint32_t counting;

static void lower_span(uint32_t span)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t table = bridge_span_table(span);
    uint32_t strength;
    uint32_t r = 0;

    while (r < count) {
        strength = BRIDGE_GET(shield_t, strength, shield, table, r);
        bridge_turn(shield, table, r, strength % 3 != 0);
        r++;
    }
}

BRIDGE_EXPORT(lower) int lower(void)
{
    uint32_t spans = bridge_count_spans(lowering);
    uint32_t s = 0;

    while (s < spans) {
        lower_span(bridge_get_span(lowering, s));
        s++;
    }
    return (0);
}

static uint32_t add_span(uint32_t span, uint32_t *on, uint32_t *sum)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t table = bridge_span_table(span);
    uint32_t r = 0;

    while (r < count) {
        if (bridge_is_on(shield, table, r)) {
            *sum += BRIDGE_GET(shield_t, strength, shield, table, r);
            (*on)++;
        }
        r++;
    }
    return (count);
}

BRIDGE_EXPORT(count) int count(void)
{
    uint32_t spans = bridge_count_spans(counting);
    uint32_t rows = 0;
    uint32_t on = 0;
    uint32_t sum = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += add_span(bridge_get_span(counting, s), &on, &sum);
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "shields", rows, on);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "strength", sum, 0);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    shield_t value = {};
    uint64_t id;

    shield = BRIDGE_COMPONENT("game:Shield", shield_t, BRIDGE_DELETE,
        BRIDGE_TOGGLE);
    terms[0].slot = shield;
    lowering = BRIDGE_ADD_SYSTEM("lower", "update", terms, 1, nullptr, 0, nullptr, 0);
    terms[0].access = BRIDGE_READ;
    counting = BRIDGE_ADD_SYSTEM("count", "present", terms, 1, nullptr, 0, nullptr, 0);
    while (value.strength < ENTITIES) {
        id = bridge_spawn();
        BRIDGE_ATTACH(id, shield, value);
        value.strength++;
    }
    return (0);
}
