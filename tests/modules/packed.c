/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "packed.h"

static uint32_t count;
static uint32_t counting;

TEST(advance)
{
    const bridge_query_t *q = test_get_query(api, counting);
    const uint32_t *span = test_get_span(api, q, 0, 1);
    count_t *c = (count_t *)(api->shared_base + span[2]);
    bridge_line_t line = { "count", 5, 0, 0 };

    c->n += 1;
    line.a0 = c->n;
    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

TEST(init)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    count_t c = {};

    count = TEST_COMPONENT(api, "packed:Count", count_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = count;
    counting = TEST_ADD_SYSTEM(api, "advance", "update", terms, 1,
        nullptr, 0);
    TEST_ATTACH(api, api->spawn(api), count, c);
    return (0);
}

TEST(reload)
{
    return (0);
}

TEST(shutdown)
{
    return (0);
}
