/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"

[[maybe_unused]] static uint32_t seeing;
static uint32_t looking;

BRIDGE_EXPORT(saw) int saw(void)
{
    return (0);
}

BRIDGE_EXPORT(look) int look(void)
{
    uint32_t spans = bridge_get_span_count(looking);
    uint32_t rows = 0;

    for (uint32_t s = 0; s < spans; s++)
        rows += bridge_get_rows(bridge_get_span(looking, s));
    BRIDGE_LOG(BRIDGE_LOG_INFO, "rows", rows, 0);
    return (0);
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
#ifndef ROWS
    BRIDGE_LOG(BRIDGE_LOG_INFO, "kept", bridge_get_trigger_count(seeing), 0);
#endif
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    terms[0].slot = BRIDGE_COMPONENT("game:L", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
#ifdef ROWS
    looking = BRIDGE_ADD_SYSTEM("look", "present", terms, 1, nullptr, 0,
        nullptr, 0);
#else
    seeing = BRIDGE_OBSERVER("saw", BRIDGE_ON_ATTACH, terms, 1, nullptr, 0,
        nullptr, 0);
#endif
    return (0);
}
