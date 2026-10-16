/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "stepper.h"

#ifdef FAIL_RELOAD
constexpr int RELOADED = 5;
#else
constexpr int RELOADED = 0;
#endif

static uint32_t tick;
static uint32_t ticking;

TEST(advance)
{
    const bridge_query_t *q = test_get_query(api, ticking);
    const uint32_t *span = test_get_span(api, q, 0, 1);
    tick_t *t = (tick_t *)(api->shared_base + span[2]);
    bridge_line_t line = { "tick", 4, 0, STEP };

    t->n += STEP;
    line.a0 = t->n;
    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

static void declare_tick(const test_api_t *api)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};

    tick = TEST_COMPONENT(api, "game:Tick", tick_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    terms[0].slot = tick;
    ticking = TEST_ADD_SYSTEM(api, "advance", "update", terms, 1,
        nullptr, 0);
}

TEST(init)
{
    tick_t t = { 100 };

    declare_tick(api);
    TEST_ATTACH(api, api->spawn(api), tick, t);
    return (0);
}

TEST(reload)
{
    declare_tick(api);
    return (RELOADED);
}

TEST(shutdown)
{
    bridge_line_t line = { "shutdown", 8, STEP, 0 };

    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}
