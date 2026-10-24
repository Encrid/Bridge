/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

#ifdef UNSTATED
constexpr uint32_t MORE_BEFORE_COUNT = 0;
#else
constexpr uint32_t MORE_BEFORE_COUNT = 1;
#endif

#ifdef CHANGING
constexpr uint32_t CHANGE_COUNT = 1;
#else
constexpr uint32_t CHANGE_COUNT = 0;
#endif

#ifdef THIRD
    #ifdef REPEATED
constexpr uint32_t AGAIN_BEFORE_COUNT = 2;
    #else
constexpr uint32_t AGAIN_BEFORE_COUNT = 1;
    #endif
#endif

static uint32_t position;
static uint32_t doubling;
static uint32_t adding;
static uint32_t showing;
#ifdef THIRD
static uint32_t adding_again;
#endif

static uint32_t get_first_table(uint32_t query)
{
    return (bridge_span_table(bridge_get_span(query, 0)));
}

BRIDGE_EXPORT(twice) int twice(void)
{
    uint32_t table = get_first_table(doubling);

    BRIDGE_SET(position_t, x, position, table, 0,
        BRIDGE_GET(position_t, x, position, table, 0) * 2.0f);
    return (0);
}

BRIDGE_EXPORT(more) int more(void)
{
    uint32_t table = get_first_table(adding);

    BRIDGE_SET(position_t, x, position, table, 0,
        BRIDGE_GET(position_t, x, position, table, 0) + 1.0f);
    return (0);
}

#ifdef THIRD
BRIDGE_EXPORT(again) int again(void)
{
    uint32_t table = get_first_table(adding_again);

    BRIDGE_SET(position_t, x, position, table, 0,
        BRIDGE_GET(position_t, x, position, table, 0) + 2.0f);
    return (0);
}
#endif

BRIDGE_EXPORT(show) int show(void)
{
    uint32_t table = get_first_table(showing);

    BRIDGE_LOG(BRIDGE_LOG_INFO, "x",
        (uint32_t)BRIDGE_GET(position_t, x, position, table, 0), 0);
    return (0);
}

static void register_all(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
#ifdef THIRD
    uint32_t after_more[2];
#endif

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    terms[0].slot = position;
    doubling = BRIDGE_ADD_SYSTEM("twice", "update", terms, 1, &position,
        CHANGE_COUNT, nullptr, 0);
    adding = BRIDGE_ADD_SYSTEM("more", "update", terms, 1, &position,
        CHANGE_COUNT, &doubling, MORE_BEFORE_COUNT);
#ifdef THIRD
    after_more[0] = adding;
    after_more[1] = adding;
    adding_again = BRIDGE_ADD_SYSTEM("again", "update", terms, 1, nullptr, 0,
        after_more, AGAIN_BEFORE_COUNT);
#endif
    terms[0].access = BRIDGE_READ;
    showing = BRIDGE_ADD_SYSTEM("show", "present", terms, 1, nullptr, 0,
        nullptr, 0);
}

BRIDGE_EXPORT(init) int init(void)
{
    position_t p = { 1.0f, 0.0f };

    register_all();
    BRIDGE_ATTACH(bridge_spawn(), position, p);
    return (0);
}

#ifdef RELOADABLE
BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    register_all();
    return (0);
}
#endif
