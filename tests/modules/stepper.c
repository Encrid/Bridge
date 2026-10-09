/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/stepper.c -- ticker.c as a shared library. A process
** cannot hold two copies of one library, so a reload of this one shuts the
** old instance down before the new one is opened. STEP is what a tick adds,
** and a build with FAIL_RELOAD refuses its reload.
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

/*
** advance
**
** Adds STEP to the count, and says what it has become.
*/
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

/*
** init
**
** Declares the component and the system, and spawns one entity whose count
** is 100.
*/
TEST(init)
{
    tick_t t = { 100 };

    declare_tick(api);
    TEST_ATTACH(api, api->spawn(api), tick, t);
    return (0);
}

/*
** reload
**
** Declares the component and the system again. It answers 0, or 5 in a
** build with FAIL_RELOAD, which is a reload that failed.
*/
TEST(reload)
{
    declare_tick(api);
    return (RELOADED);
}

/*
** shutdown
**
** Says that it is shut down, with the STEP of this build.
*/
TEST(shutdown)
{
    bridge_line_t line = { "shutdown", 8, STEP, 0 };

    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}
