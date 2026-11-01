/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

static uint32_t one;
static uint32_t two;

BRIDGE_EXPORT(first) int first(void)
{
    bridge_attach(bridge_spawn(), one, BRIDGE_ZEROED);
    return (0);
}

BRIDGE_EXPORT(second) int second(void)
{
    bridge_attach(bridge_spawn(), two, BRIDGE_ZEROED);
    return (0);
}

#ifdef FIRST_OPER

static uint32_t walking;

BRIDGE_EXPORT(walk) int walk(void)
{
    uint32_t spans = bridge_get_span_count(walking);
    uint32_t rows = 0;
    uint32_t s = 0;

    while (s < spans) {
        rows += bridge_get_rows(bridge_get_span(walking, s));
        s++;
    }
    BRIDGE_LOG(BRIDGE_LOG_INFO, "walked", spans, rows);
    return (0);
}

static void add_walk(void)
{
    bridge_term_t both[2] = {{ 0, BRIDGE_READ, FIRST_OPER, 0 },
        { 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    both[0].slot = one;
    both[1].slot = two;
    walking = BRIDGE_ADD_SYSTEM("walk", "present", both, 2, nullptr, 0,
        nullptr, 0);
}

#endif

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    one = BRIDGE_DECLARE_TAG("game:One", BRIDGE_STRIP, BRIDGE_TABLE);
    two = BRIDGE_DECLARE_TAG("game:Two", BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_ADD_SYSTEM("first", "update", &none, 0, &one, 1, nullptr, 0);
    BRIDGE_ADD_SYSTEM("second", "update", &none, 0, &two, 1, nullptr, 0);
#ifdef FIRST_OPER
    add_walk();
#endif
    return (0);
}
