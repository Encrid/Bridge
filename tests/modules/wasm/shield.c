/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/shield.c -- a toggled component on the wasm backend:
** its bits found ahead of its column, read, and written.
**
** storage.md, *A toggle*
*/

#include "bridge_wasm.h"
#include "shield.h"
#include "wasm_idle.h"

constexpr uint32_t ENTITIES = 10;

static uint32_t shield;
static uint32_t lowering;
static uint32_t counting;

/*
** lower_span
**
** Turns off every shield of one span whose strength is a multiple of
** three, and turns every other one on.
*/
static void lower_span(uint32_t span)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t base = bridge_get_base(span, 0);
    uint32_t bits = bridge_get_bits(base);
    uint32_t strength;
    uint32_t r = 0;

    while (r < count) {
        strength = BRIDGE_GET(shield_t, strength,
            BRIDGE_ROW(shield_t, base, r, count));
        bridge_turn(bits, r, count, strength % 3 != 0);
        r++;
    }
}

/*
** lower
**
** Turns every third shield off: a write of its bit, by a system that
** stated the term as written.
*/
BRIDGE_EXPORT(lower) int lower(void)
{
    uint32_t spans = bridge_count_spans(lowering);
    uint32_t s = 0;

    while (s < spans) {
        lower_span(bridge_get_span(lowering, s, 1));
        s++;
    }
    return (0);
}

/*
** add_span
**
** Adds one to `on` and the strength to `sum` for every shield of one span
** that is on, and answers how many rows the span has.
*/
static uint32_t add_span(uint32_t span, uint32_t *on, uint32_t *sum)
{
    uint32_t count = bridge_count_rows(span);
    uint32_t base = bridge_get_base(span, 0);
    uint32_t bits = bridge_get_bits(base);
    uint32_t r = 0;

    while (r < count) {
        if (bridge_is_on(bits, r, count)) {
            *sum += BRIDGE_GET(shield_t, strength,
                BRIDGE_ROW(shield_t, base, r, count));
            (*on)++;
        }
        r++;
    }
    return (count);
}

/*
** count
**
** Says how many shields there are, how many are on, and the sum of the
** strengths of those.
*/
BRIDGE_EXPORT(count) int count(void)
{
    uint32_t spans = bridge_count_spans(counting);
    uint32_t rows = 0;
    uint32_t on = 0;
    uint32_t sum = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += add_span(bridge_get_span(counting, s, 1), &on, &sum);
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "shields", rows, on);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "strength", sum, 0);
    return (0);
}

/*
** init
**
** Declares the component as a toggle, the system that writes its bits and
** the one that reads them, and spawns the entities, each with a shield of
** its own number as strength.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    shield_t value = {};
    uint64_t id;

    shield = BRIDGE_COMPONENT("game:Shield", shield_t, BRIDGE_DELETE,
        BRIDGE_TOGGLE);
    terms[0].slot = shield;
    lowering = BRIDGE_ADD_SYSTEM("lower", "update", terms, 1, nullptr, 0);
    terms[0].access = BRIDGE_READ;
    counting = BRIDGE_ADD_SYSTEM("count", "present", terms, 1, nullptr, 0);
    while (value.strength < ENTITIES) {
        id = bridge_spawn();
        BRIDGE_ATTACH(id, shield, value);
        value.strength++;
    }
    return (0);
}
