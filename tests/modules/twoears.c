/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

static uint32_t sides[2];

BRIDGE_EXPORT(left_heard) int left_heard(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "heard", 0, 0);
    return (0);
}

BRIDGE_EXPORT(right_heard) int right_heard(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "heard", 0, 0);
    return (0);
}

BRIDGE_EXPORT(touch) int touch(void)
{
    marked_t value = { 1 };
    uint64_t e = bridge_spawn();

    BRIDGE_ATTACH(e, sides[0], value);
#ifdef BOTH
    BRIDGE_ATTACH(e, sides[1], value);
#endif
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    sides[0] = BRIDGE_COMPONENT("game:L", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    sides[1] = BRIDGE_COMPONENT("game:R", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = sides[0];
    BRIDGE_OBSERVER("left_heard", BRIDGE_ON_ATTACH, terms, 1, nullptr, 0,
        nullptr, 0);
    terms[0].slot = sides[1];
    BRIDGE_OBSERVER("right_heard", BRIDGE_ON_ATTACH, terms, 1, nullptr, 0,
        nullptr, 0);
#ifndef EARS
    BRIDGE_ADD_SYSTEM("touch", "update", nullptr, 0, sides, 2, nullptr, 0);
#endif
    return (0);
}
