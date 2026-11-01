/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "game.h"
#include "idle.h"
#include "reach.h"
#include "say.h"

#ifdef UNSTATED
constexpr uint32_t STATED = 0;
#else
constexpr uint32_t STATED = 1;
#endif

static uint32_t position;
static uint32_t marked;
static uint32_t stamping;

#ifdef LEFT

static uint32_t get_left(const test_api_t *api)
{
    return (test_get_header(api)->changes_left);
}

TEST(stamp)
{
    const bridge_query_t *q = test_get_query(api, stamping);
    const uint32_t *span = test_get_span(api, q, 0, 1);
    const uint64_t *who = (const uint64_t *)(api->shared_base
        + get_blocks(api)[span[0]]);
    marked_t m = { VALUE };
    uint32_t start = get_left(api);
    uint32_t made = 0;

    while (get_left(api) >= 32) {
        TEST_ATTACH(api, who[0], marked, m);
        made++;
    }
    say(api, "stamp left", start, made);
    return (0);
}

#else

TEST(stamp)
{
    const bridge_query_t *q = test_get_query(api, stamping);
    const uint32_t *blocks = get_blocks(api);
    marked_t m = { VALUE };
    const uint32_t *span;
    const uint64_t *who;
    uint32_t s = 0;
    uint32_t row;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        who = (const uint64_t *)(api->shared_base + blocks[span[0]]);
        row = 0;
        while (row < test_get_rows(api, span)) {
            TEST_ATTACH(api, who[row], marked, m);
            row++;
        }
        s++;
    }
    return (0);
}

#endif

TEST(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    position_t p = { 1.0f, 0.0f };
#ifdef LEFT
    uint32_t start = get_left(api);
#endif

    position = TEST_COMPONENT(api, "game:Position", position_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    marked = TEST_COMPONENT(api, "game:Marked", marked_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = position;
    stamping = TEST_ADD_SYSTEM(api, "stamp", PHASE, terms, 1,
        &marked, STATED, nullptr, 0);
    TEST_ATTACH(api, api->spawn(api), position, p);
#ifdef LEFT
    say(api, "init left", start, get_left(api));
#endif
    return (0);
}
