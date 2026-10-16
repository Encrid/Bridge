/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

static uint32_t single;
static uint32_t hearing;

BRIDGE_EXPORT(heard) int heard(void)
{
    uint32_t at = bridge_get_single(single);

    BRIDGE_LOG(BRIDGE_LOG_INFO, "heard", bridge_count_triggers(hearing),
        at ? BRIDGE_GET(marked_t, by, at) : 0);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    single = BRIDGE_COMPONENT("game:S", marked_t, BRIDGE_STRIP,
        BRIDGE_SINGLETON);
    terms[0].slot = single;
    hearing = BRIDGE_OBSERVER("heard", BRIDGE_ON_ATTACH, terms, 1, nullptr,
        0);
    return (0);
}
