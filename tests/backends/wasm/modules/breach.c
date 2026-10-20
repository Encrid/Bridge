/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

constexpr uint32_t BREACH_ENTITIES = 4;

static uint32_t position;
static uint32_t moving;
#if defined(BREACH_SPARSE)
static uint32_t marked;
#endif
#if defined(BREACH_DIRECT)
static uint32_t dotted;
#endif

#if defined(BREACH_ROW) || defined(BREACH_OK)
static uint32_t a_table(void)
{
    return (bridge_span_table(bridge_get_span(moving, 0)));
}
#endif

BRIDGE_EXPORT(breach) int breach(void)
{
#if defined(BREACH_SLOT)
    (void)acc_ld_32(50, 0, 0);
#elif defined(BREACH_ROW)
    (void)BRIDGE_GET(position_t, x, position, a_table(), 1u << 20);
#elif defined(BREACH_FIELD)
    acc_st_32(position, BRIDGE_AT(0, 0), 1u << 20, 0);
#elif defined(BREACH_READONLY)
    acc_st_32(position, BRIDGE_AT(0, 0), 0, 0);
#elif defined(BREACH_FORGE)
    acc_own_st(BRIDGE_HEADER(queries), 0);
#elif defined(BREACH_SPARSE)
    (void)BRIDGE_GET(marked_t, by, marked, 1u << 20, 0);
#elif defined(BREACH_DIRECT)
    (void)BRIDGE_GET(marked_t, by, dotted, 1u << 20, 0);
#elif defined(BREACH_TO_SHARED)
    uint32_t word = 0;

    acc_to_shared(0, &word, sizeof(word));
#elif defined(BREACH_FROM_SHARED)
    uint32_t word = 0;

    acc_from_shared(&word, 0, sizeof(word));
    (void)word;
#elif defined(BREACH_OK)
    (void)BRIDGE_GET(position_t, x, position, a_table(), 0);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "ok", BREACH_ENTITIES, 0);
#endif
    return (0);
}

static void populate(void)
{
    position_t p = { 0.0f, 0.0f };
    uint32_t i = 0;

    while (i < BREACH_ENTITIES) {
        BRIDGE_ATTACH(bridge_spawn(), position, p);
        i++;
    }
}

BRIDGE_EXPORT(init) int init(void)
{
#if defined(BREACH_FIELD)
    bridge_term_t term[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
#else
    bridge_term_t term[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
#endif

    position = BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
#if defined(BREACH_SPARSE)
    marked = BRIDGE_COMPONENT("game:Mark", marked_t, BRIDGE_DELETE,
        BRIDGE_SPARSE_SET);
#endif
#if defined(BREACH_DIRECT)
    dotted = BRIDGE_COMPONENT("game:Dot", marked_t, BRIDGE_DELETE,
        BRIDGE_DIRECT_INDEXED);
#endif
    term[0].slot = position;
    moving = BRIDGE_ADD_SYSTEM("breach", "update", term, 1, nullptr, 0,
        nullptr, 0);
    populate();
    return (0);
}
