/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "game.h"
#include "idle.h"

#ifdef UNSTATED
constexpr uint32_t STATED = 0;
#else
constexpr uint32_t STATED = 1;
#endif

static uint32_t position;
static uint32_t marked;
static uint32_t stamping;

TEST(stamp)
{
    const bridge_query_t *q = test_get_query(api, stamping);
    marked_t m = { VALUE };
    const uint32_t *span;
    const uint64_t *who;
    uint32_t s = 0;
    uint32_t row;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        who = (const uint64_t *)(api->shared_base + span[1]);
        row = 0;
        while (row < span[0]) {
            TEST_ATTACH(api, who[row], marked, m);
            row++;
        }
        s++;
    }
    return (0);
}

TEST(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    position_t p = { 1.0f, 0.0f };

    position = TEST_COMPONENT(api, "game:Position", position_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    marked = TEST_COMPONENT(api, "game:Marked", marked_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = position;
    stamping = TEST_ADD_SYSTEM(api, "stamp", PHASE, terms, 1,
        &marked, STATED, nullptr, 0);
    TEST_ATTACH(api, api->spawn(api), position, p);
    return (0);
}
